/**
 * @file pdcch_dump.c
 * @brief PDCCH buffer dumping
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "pdcch_dump.h"

#define PDCCH_DUMP_RECORD_VERSION 0
#define PDCCH_ADD_CELL_INFO_RECORD_VERSION 2

/* PDCCH dump service client pointer, NULL when no client is set */
void* pdcch_dump_svc_client;
/* PDCCH cell info service client pointer, NULL when no client is set */
void* pdcch_cell_info_svc_client;

unsigned int pdcch_dump_thread_id = 0;
unsigned int pdcch_cell_info_thread_id = 0;

/* semaphore on which the cell info thread waits */
unsigned int pdcch_cell_info_thread_semaphore;

/* handle of "mpll1_out_early_div5" clock (needed to test if access is allowed) */
void* clk_handle_mpll1_out_early_div5 = 0;

/* last observed LTE phy cell ID */
int cur_cell_id = -1;
/* last observed LTE system bandwidth (downlink), antenna numbers and PHICH Ng, EARFCN */
unsigned char cur_sys_bandwidth;
unsigned int cur_num_txant;
unsigned int cur_num_rxant;
unsigned char cur_num_phich_group;
unsigned short cur_earfcn = 0;

/******** metadata collection hooks ********/

__attribute__ ((overwrite ("pdcch_demback_init")))
void pdcch_demback_init_hook(unsigned int subframe, unsigned int frame, unsigned int carrier_index) {
    unsigned char* cell_carrier_struct = lte_LL1_get_cell_info(carrier_index);
    unsigned short cell_id = *((unsigned short*)(cell_carrier_struct + 0x12));
    int cur_cell_id_new = cell_id & 0x1FF;
    cur_sys_bandwidth = *(cell_carrier_struct + 0xC) & 0x7;
    
    pdcch_demback_init_fw_org(subframe, frame, carrier_index);   
    
    if (pdcch_cell_info_thread_id != 0) {
        if (cur_cell_id_new != cur_cell_id) {
            cur_cell_id = cur_cell_id_new;
            sem_post(&pdcch_cell_info_thread_semaphore);
        }
    }
    cur_cell_id = cur_cell_id_new;
}

__attribute__ ((overwrite ("_demback_pdcchi_sth")))
void _demback_pdcchi_sth_hook(unsigned char* config_struct, unsigned int carrier_index) {
    cur_num_phich_group = *(config_struct + 0x12);
    
    _demback_pdcchi_sth_fw_org(config_struct, carrier_index);
}

__attribute__ ((overwrite ("lte_LL1_csf_callback")))
void lte_LL1_csf_callback_hook(unsigned int carrier_index, unsigned int system_frame_number, unsigned int sub_frame_number) {
    lte_LL1_csf_callback_fw_org(carrier_index, system_frame_number, sub_frame_number);
    
    cur_num_txant = *((unsigned short*)(antenna_config_struct_ptr + 0x80));
    cur_num_rxant = *((unsigned short*)(antenna_config_struct_ptr + 0x7E));
}

__attribute__ ((overwrite ("_pbch_decode_req_received")))
void _pbch_decode_req_received_hook(unsigned int u0, unsigned char* lte_LL1_dl_pbch_decode_req) {
    _pbch_decode_req_received_fw_org(u0, lte_LL1_dl_pbch_decode_req);
    
    if (*(lte_LL1_dl_pbch_decode_req + 0x14)) { //check if request is for serving cell
        cur_earfcn = *((unsigned short*)(lte_LL1_dl_pbch_decode_req + 0x10));
    }
}

/******** determine clock handle ("mpll1_out_early_div5") ********/

__attribute__ ((overwrite ("HAL_clk_EnableClock")))
void HAL_clk_EnableClock_hook(void* pClockHandle) {
    HAL_clk_EnableClock_fw_org(pClockHandle);
    
    if (pClockHandle != 0) {
        char* clock_name;
        HAL_clk_GetClockName(pClockHandle, &clock_name);
        
        if (clk_handle_mpll1_out_early_div5 == 0) {
            if (strcmp(clock_name, "mpll1_out_early_div5") == 0) {
                clk_handle_mpll1_out_early_div5 = pClockHandle;
            }
        }
    }
}

/******** actual dump collection threads ********/

__attribute__ ((noinline))
void thread_loop_wait(unsigned int wait_time) {
    volatile unsigned int i;
    for (i = 0; i < wait_time; i++) {
    }
}

/**
 * reads a hardware peripheral register and return its value
 * 
 * This function is needed to prevent the compiler from optimizing accesses,
 * e.g. when it is just before checked if the peripheral clock is enabled
 * (the compiler might then already read the hw register while jumping conditionally,
 * so also in case the if evaluates to false)
 */
__attribute__ ((noinline))
unsigned int read_hw_reg_safe(volatile unsigned int* hw_reg_ptr) {
    return *hw_reg_ptr;
}

void pdcch_dump_thread_main() {
    pthread_setschedprio(pthread_self(), 2); //set a low priority for our thread, such that we do not interfer with the modem's normal operation (only sleep has an even lower priority)
    
    unsigned int brdg_cfg[2] = {
        (0b0110 << 25) | (7 << 19) | (0xBA0), //CFG0 Q_SIZE (Q_SIZE_32), MAX_BANK, VBUF_LEN
        (0x1127)  //CFG1: set START_LINE, rest 0 
    };
    unsigned int last_dumped_frame_number = 10; //set to value that will never occur (as subframe is 0..9)
    
    // reserve memory for indication message
    test_data_ind_msg_v01* pdcch_dump_ind = (test_data_ind_msg_v01*)malloc(sizeof(test_data_ind_msg_v01));
    memset(pdcch_dump_ind, 0, sizeof(test_data_ind_msg_v01));
    unsigned int* const data32 = (unsigned int*)(pdcch_dump_ind->data);
    *data32 = PDCCH_DUMP_SVC_ID;
    *(data32 + 1) = PDCCH_DUMP_RECORD_VERSION;
    const unsigned int header_length = 3;
    
    while (1) {      
        // check that the main peripheral clock is running, otherwise we cannot read any hardware register
        // (well, we can, but the modem will crash)
        // it is safe to call HAL_clk_IsClockEnabled with a NULL pointer, just returns "false"
        if (HAL_clk_IsClockOn(clk_handle_mpll1_out_early_div5)) {
            // check that demodulation backend is powered on (Bit0 of MODEM_PWRUP)
            if (read_hw_reg_safe(&MODEM_PWRUP) & 1) {
                unsigned int frame_number_reg_val = read_hw_reg_safe(&PDCCH_REG_REORDER_WORD0);
                unsigned int frame_number = (frame_number_reg_val) & 0x3FFF; //[13:4] frame number; [3:0] subframe number
                if ((frame_number_reg_val != 0) && (frame_number != last_dumped_frame_number)) {
                    unsigned int num_rb = (PDCCH_DEINT_CFG_WORD1) & 0xFF;
                    unsigned int cfi_detected = (LTE_DBE_PCFICH_STATUS) & 0xC0000000;
                    unsigned int reg_heights = PDCCH_DEINT_CFG_WORD1;
                    unsigned int num_regs_per_rb = ((reg_heights >> 20) & 0x3);
                    if (cfi_detected >= 1) {
                        num_regs_per_rb += ((reg_heights >> 22) & 0x3);
                    }
                    if (cfi_detected == 2) {
                        num_regs_per_rb += ((reg_heights >> 24) & 0x3);
                    }
                    unsigned int buffer_dump_length = num_regs_per_rb * num_rb + 1; //+1 because of header in LLR buffer
                    buffer_dump_length += buffer_dump_length/2 + 1; //floor(buffer_dump_length*1,5) + 1
                    
                    //save old BRDG configuration
                    unsigned int brdg_cfg_bak[3];
                    brdg_cfg_bak[0] = MEM_PL_BRDG_CH0_CFG0;
                    brdg_cfg_bak[1] = MEM_PL_BRDG_CH0_CFG1;
                    brdg_cfg_bak[2] = MEM_PL_MEM_CH0_PAGE_NUM;
                    
                    //configure BRDG for our buffer and copy data
                    mempool_config_ch_and_brdg(0, brdg_cfg);
                    mempool_cpy_page(0, 0, (unsigned int*)(data32 + 1 + header_length), buffer_dump_length);
                    
                    //reconfigure the BRDG, if we do not do this, no DCIs are received anymore! seems crucial this BRDG..
                    MEM_PL_BRDG_CH0_CFG0 = brdg_cfg_bak[0];
                    MEM_PL_BRDG_CH0_CFG1 = brdg_cfg_bak[1];
                    MEM_PL_MEM_CH0_PAGE_NUM = brdg_cfg_bak[2];
                    
                    // [31:30] CFI
                    // [24:16] cell ID (phy)
                    // [13:4]  frame number
                    // [3:0]   subframe number
                    *(data32 + 2) = frame_number | cfi_detected | (cur_cell_id << 16);
                    unsigned int phich_duration = (PDCCH_REG_REORDER_WORD1) & 0x3;
                    unsigned int crnti = (TBVD_CCH_LTE_CFG_WORD4) & 0xFFFF;
                    // [31:16] UE C-RNTI
                    // [15:10] number of PHICH groups
                    // [9:8] PHICH duration
                    // [7:0] total number of RBs/symbol (bandwidth)
                    *(data32 + 3) = (crnti << 16) | num_rb | (phich_duration << 8) | ((cur_num_phich_group & 0x3F) << 10);

                    //send indication message to client
                    pdcch_dump_ind->data_len = (1 + header_length + buffer_dump_length) << 2;
                    qmi_csi_send_ind(pdcch_dump_svc_client, QMI_TEST_DATA_IND_V01, pdcch_dump_ind, sizeof(test_data_ind_msg_v01));
                                        
                    last_dumped_frame_number = frame_number;
                }
            }
        }
        
        thread_loop_wait(2048*2); //TODO wait with semaphore for good moment instead of polling
        if (pdcch_dump_svc_client == 0) {
            pdcch_dump_thread_id = 0;
            break;
        }
    }
    
    free(pdcch_dump_ind);
}

unsigned char* peri_reg_read(unsigned char* buffer, volatile unsigned int* peri_reg_ptr, unsigned int length) {
    unsigned int i;
    for (i = 0; i < length/4; i++) {
        *(((unsigned int*)(buffer)) + i) = *(peri_reg_ptr + i);
    }
    return buffer + length;
}

void pdcch_cell_info_thread_main() {
    pthread_setschedprio(pthread_self(), 2);
    
    test_data_ind_msg_v01* pdcch_cell_info_ind = (test_data_ind_msg_v01*)malloc(sizeof(test_data_ind_msg_v01));
    memset(pdcch_cell_info_ind, 0, sizeof(test_data_ind_msg_v01));
    unsigned int* const data32 = (unsigned int*)(pdcch_cell_info_ind->data);
    *data32 = PDCCH_CELL_INFO_SVC_ID;
    *(data32 + 1) = PDCCH_ADD_CELL_INFO_RECORD_VERSION;
    
    while (1) {
        sem_wait(&pdcch_cell_info_thread_semaphore);
        if (pdcch_cell_info_svc_client == 0) {
            pdcch_cell_info_thread_id = 0;
            break;
        }
        
        while (1) { //wait for modem to be in a state where we can read all relevant registers (see dump thread)
            if (HAL_clk_IsClockOn(clk_handle_mpll1_out_early_div5)) {
                if (read_hw_reg_safe(&MODEM_PWRUP) & 1) {
                    if (read_hw_reg_safe(&PDCCH_REG_REORDER_WORD0) != 0) {                         
                        // [18:16] System bandwidth
                        // [13:11] number of TX antennas
                        // [10:9] number of RX antennas
                        // [8:0] phy cell ID
                        *(data32 + 2) = cur_cell_id | (cur_sys_bandwidth << 16) | (((cur_num_txant-1) & 0x7) << 11) | (((cur_num_rxant-1) & 0x3) << 9);

                        unsigned char* buf_pos = pdcch_cell_info_ind->data + 12;
                        buf_pos = peri_reg_read(buf_pos, &PDCCH_DEINT_CFG_WORD1, 8);
                        
                        buf_pos = peri_reg_read(buf_pos, &SYMBOL0_REG_MAP_0_WORD1, 28);
                        buf_pos = peri_reg_read(buf_pos, &SYMBOL0_REG_MAP_1_WORD1, 28);
                        buf_pos = peri_reg_read(buf_pos, &SYMBOL0_REG_MAP_2_WORD1, 28);
                        buf_pos = peri_reg_read(buf_pos, &SYMBOL1_REG_MAP_0_WORD1, 40);
                        buf_pos = peri_reg_read(buf_pos, &SYMBOL1_REG_MAP_1_WORD1, 40);
                        buf_pos = peri_reg_read(buf_pos, &SYMBOL2_REG_MAP_WORD1, 40);
                        
                        buf_pos = peri_reg_read(buf_pos, &PDCCH_REG_REORDER_WORD1, 16);
                        
                        buf_pos = peri_reg_read(buf_pos, &TBVD_CCH_LTE_CFG_WORD4, 32); //RNTI configuration
                        
                        // [15:0] EARFCN (DL)
                        *(data32 + 68) = cur_earfcn;
                        
                        //send indication message to client
                        pdcch_cell_info_ind->data_len = (4 + 4 + 4 + 260 + 4);
                        qmi_csi_send_ind(pdcch_cell_info_svc_client, QMI_TEST_DATA_IND_V01, pdcch_cell_info_ind, sizeof(test_data_ind_msg_v01));
                        
                        break;
                    }
                }
            }
            thread_loop_wait(2048*2);
        }
    }
}

/******** QMI message handlers ********/

/**
 * @brief PDCCH dump service register request handler
 */
int pdcch_dump_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    if (*req_data != 0) {        
        pdcch_dump_svc_client = *((void**)clnt_info);
        *((void**)(resp_data)) = pdcch_dump_svc_client;
        
        if (pdcch_dump_thread_id == 0) {
//             pthread_attr_t attr;
//             pthread_attr_init(&attr);
//             pthread_attr_setthreadname(&attr, "pddch_dump_thread");
//             pthread_create(&pdcch_dump_thread_id, &attr, &pdcch_dump_thread_main, 0);
            pthread_create(&pdcch_dump_thread_id, 0, &pdcch_dump_thread_main, 0);
        }
        
        return 4;
    } else {
        pdcch_dump_svc_client = 0;
        return 0;
    }
    
    return 0;
}

/**
 * @brief PDCCH cell info service register request handler
 */
int pdcch_cell_info_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    if (*req_data != 0) {        
        pdcch_cell_info_svc_client = *((void**)clnt_info);
        *((void**)(resp_data)) = pdcch_cell_info_svc_client;
        
        if (pdcch_cell_info_thread_id == 0) {
            sem_init(&pdcch_cell_info_thread_semaphore, 0, 0);
            pthread_create(&pdcch_cell_info_thread_id, 0, &pdcch_cell_info_thread_main, 0);
        }
        sem_post(&pdcch_cell_info_thread_semaphore);
        
        return 4;
    } else {
        pdcch_cell_info_svc_client = 0;
        return 0;
    }
    
    return 0;
}

