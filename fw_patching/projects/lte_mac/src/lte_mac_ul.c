/**
 * @file lte_mac_ul.c
 * @brief LTE MAC Layer messages forwarding (over QMI) patches - uplink part
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "lte_mac.h" 

#define PHY_TASK_QUEUE_SIZE 64

/**
 * @brief enumeration for different tasks types used to describe the MAC PDU to send
 */
typedef enum {
    GATHER_OR_FILL_TASK,
    DSM_TASK,
    PADDING_TASK
} phy_task_type_t;

/**
 * @brief structure to describe a task (which is used to describe the MAC PDU to send)
 */
typedef struct {
    phy_task_type_t type;
    void* src_address;
    unsigned int length_in_bytes;
    unsigned int dsm_offset;
    void* task_ptr;
} phy_task_t;

/* queue implementation which holds a set of tasks to describe a MAC PDU */
phy_task_t phy_task_queue[PHY_TASK_QUEUE_SIZE];
unsigned int phy_task_queue_count = 0; //number of elements currently in the queue

/* flag indication whether or not the PHY is in LTE mode */
unsigned int phy_lte_mode = 0;

/* last time and harq values, to send them in the QMI indication later */
unsigned char last_harq_id;
unsigned int last_univ_stmr_val;

/* LTE MAC client pointer, NULL when no client is set */
extern void* lte_mac_svc_client;

/* structure used to build QMI indications */
extern test_data_ind_msg_v01 indication_buf;

/* RACH preamble send attempt counter */
unsigned char ul_rach_attempt = 0;

/**
 * @brief PHY technology set function, used to detect LTE mode
 * 
 * @param technology new technology mode of the PHY, e.g. LTE or UMTS
 */
__attribute__ ((overwrite ("a2_ul_phy_set_technology")))
void a2_ul_phy_set_technology_hook(unsigned int technology) {
    a2_ul_phy_set_technology_fw_org(technology);
    phy_lte_mode = (technology == 2);
}

/**
 * @brief first init function, used to get HARQ id and time (to derive frame number)
 * 
 * @param harq_id HARQ ID of the frame
 * @param univ_stmr_val time at which the frame should be sent, 19200 ticks equal one LTE subframe (1ms)
 * remark: changing this value here has no effect on the actual send time of the frame!
 */
__attribute__ ((overwrite ("a2_ul_phy_harq_init")))
void a2_ul_phy_harq_init_hook(unsigned char harq_id, unsigned int univ_stmr_val) {
    phy_task_queue_count = 0;
    last_harq_id = harq_id;
    
    last_univ_stmr_val = univ_stmr_val;
    a2_ul_phy_harq_init_fw_org(harq_id, univ_stmr_val);
}

/**
 * @brief adds a new task to the task queue (our implementation, not the one in the PHY layer)
 * 
 * @param src_address data source address or pointer to DSM element
 * @param length_in_bytes data length in length
 * @param dsm_offset just used for a DSM task: offset of data from the beginning of the DSM element
 * @param task_ptr pointer to uniquely identify the task, needed to modify it later
 */
void add_task(phy_task_type_t type, void* src_address, unsigned int length_in_bytes, unsigned int dsm_offset, void* task_ptr) {
    if ((phy_lte_mode) && (phy_task_queue_count < PHY_TASK_QUEUE_SIZE)) {
        phy_task_queue[phy_task_queue_count].type = type;
        phy_task_queue[phy_task_queue_count].src_address = src_address;
        phy_task_queue[phy_task_queue_count].length_in_bytes = length_in_bytes;
        phy_task_queue[phy_task_queue_count].dsm_offset = dsm_offset;
        phy_task_queue[phy_task_queue_count].task_ptr = task_ptr;
        phy_task_queue_count++;
    }
}

/**
 * @brief modifies a task in the task queue (our implementation, not the one in the PHY layer)
 * 
 * @param src_address data source address
 * @param length_in_bytes data length in length
 * @param task_ptr pointer to uniquely identify the task which was assigned in the add_task function
 */
void modify_task(void* src_address, unsigned int length_in_bytes, void* task_ptr) {
    int i;
    for (i = 0; i < phy_task_queue_count; i++) {
        if (task_ptr == phy_task_queue[i].task_ptr) {
            phy_task_queue[i].src_address = src_address;
            phy_task_queue[i].length_in_bytes = length_in_bytes;
            return;
        }
    }
}

/* **************** reserve task hooks **************** */
/*
 * These functions reserve a space in the task queue which then later can be filled with data.
 * This is neccessary to achieve the correct order of data even when we write earlier parts later,
 * e.g. the MAC header after the SDU.
 */

__attribute__ ((overwrite ("a2_ul_phy_reserve_gather_task")))
void* a2_ul_phy_reserve_gather_task_hook(void) {
    void* task = a2_ul_phy_reserve_gather_task_fw_org();
    add_task(GATHER_OR_FILL_TASK, 0, 0, 0, task);
    return task;
}

__attribute__ ((overwrite ("a2_ul_phy_reserve_first_gather_task")))
void* a2_ul_phy_reserve_first_gather_task_hook(void) {
    void* task = a2_ul_phy_reserve_first_gather_task_fw_org();
    add_task(GATHER_OR_FILL_TASK, 0, 0, 0, task);
    return task;
}

__attribute__ ((overwrite ("a2_ul_phy_reserve_first_fill_task")))
void* a2_ul_phy_reserve_first_fill_task_hook(void) {
    void* task = a2_ul_phy_reserve_first_fill_task_fw_org();
    add_task(GATHER_OR_FILL_TASK, 0, 0, 0, task);
    return task;
}

/* **************** write task hooks **************** */
/*
 * These functions write tasks which define data bytes of our PDU.
 * Some of them modify a reserved task and have a parameter "ul_phy_task_q_addr" 
 * which should be set to the return value of the reserve function.
 * 
 * We do not need to hook the remaining write functions because they 
 * are either not used or internally call one of these functions
 */

__attribute__ ((overwrite ("a2_ul_phy_write_gather_task_to_specified_addr")))
void a2_ul_phy_write_gather_task_to_specified_addr_hook(void* ul_phy_task_q_addr, void* source_address, unsigned short length_in_bits) {
    a2_ul_phy_write_gather_task_to_specified_addr_fw_org(ul_phy_task_q_addr, source_address, length_in_bits);
    modify_task(source_address, length_in_bits >> 3, ul_phy_task_q_addr);
}

__attribute__ ((overwrite ("a2_ul_phy_write_first_gather_task_to_specified_addr")))
void a2_ul_phy_write_first_gather_task_to_specified_addr_hook(void* ul_phy_task_q_addr, void* source_address, void* dest_address, unsigned short dst_mem_type_p, unsigned short length_in_bits) {
    a2_ul_phy_write_first_gather_task_to_specified_addr_fw_org(ul_phy_task_q_addr, source_address, dest_address, dst_mem_type_p, length_in_bits);
    modify_task(source_address, length_in_bits >> 3, ul_phy_task_q_addr);
}

__attribute__ ((overwrite ("a2_ul_phy_write_first_fill_task_to_specified_addr")))
void a2_ul_phy_write_first_fill_task_to_specified_addr_hook(void* ul_phy_task_q_addr, void* source_address, void* dest_address, unsigned short dst_mem_type_p, unsigned short length_in_bits) {
    a2_ul_phy_write_first_fill_task_to_specified_addr_fw_org(ul_phy_task_q_addr, source_address, dest_address, dst_mem_type_p, length_in_bits);
    modify_task(source_address, length_in_bits >> 3, ul_phy_task_q_addr);
}

__attribute__ ((overwrite ("a2_ul_phy_write_dsm_gather_task")))
void a2_ul_phy_write_dsm_gather_task_hook(void* pkt_ptr, unsigned short offset_in_bytes, unsigned short length_in_bytes) {
    a2_ul_phy_write_dsm_gather_task_fw_org(pkt_ptr, offset_in_bytes, length_in_bytes);
    add_task(DSM_TASK, pkt_ptr, length_in_bytes, offset_in_bytes, 0);
}

__attribute__ ((overwrite ("a2_ul_phy_write_fill_task")))
void a2_ul_phy_write_fill_task_hook(void* source_address, unsigned short length_in_bits) {
    a2_ul_phy_write_fill_task_fw_org(source_address, length_in_bits);
    add_task(GATHER_OR_FILL_TASK, source_address, length_in_bits >> 3, 0, 0);
}

__attribute__ ((overwrite ("a2_ul_phy_write_padding_bits")))
void a2_ul_phy_write_padding_bits_hook(unsigned short num_pad_bits) {
    a2_ul_phy_write_padding_bits_fw_org(num_pad_bits);
    add_task(PADDING_TASK, 0, num_pad_bits >> 3, 0, 0);
}

/* **************** done task hook **************** */

/**
 * @brief done task which is called when all data is ready and the frame can be send
 * 
 * The hook function rebuilds the MAC PDU and sends it over QMI
 * 
 * @param harq_id HARQ ID of the frame
 * @param tb_size_in_bits total PDU/transport block size in bits (to check)
 * @param interrupt_reqd flag indicating if an interrupt is required
 */
__attribute__ ((overwrite ("a2_ul_phy_write_done_task")))
void a2_ul_phy_write_done_task_hook(unsigned char harq_id, unsigned short tb_size_in_bits, unsigned int interrupt_reqd) {     
    if ((phy_lte_mode) && (lte_mac_svc_client != 0)) {
        test_data_ind_msg_v01* ind = &indication_buf;
        memset(ind, 0, sizeof(test_data_ind_msg_v01));
        unsigned int* data32 = (unsigned int*)(ind->data);
        *data32 = LTE_MAC_UL_SVC_ID;
        
        *(data32 + 1) =  last_univ_stmr_val;
        *(ind->data + 8) = last_harq_id;
        
        unsigned char* data_ptr = ind->data + 12;
        unsigned int len = 0;
        
        int i;
        for (i = 0; i < phy_task_queue_count; i++) {
            int task_bytes = phy_task_queue[i].length_in_bytes;
            if (len + task_bytes > TEST_MED_DATA_SIZE_V01-12) {
                break;
            }
            switch (phy_task_queue[i].type) {
                case GATHER_OR_FILL_TASK:
                    memcpy(data_ptr, phy_task_queue[i].src_address, task_bytes);
                    break;
                case DSM_TASK:
                    dsm_extract(phy_task_queue[i].src_address, phy_task_queue[i].dsm_offset, data_ptr, task_bytes);
                    break;
                case PADDING_TASK:
                    memset(data_ptr, 0, task_bytes);
                    break;
            }
            data_ptr += task_bytes;
            len += task_bytes;
        }
        
        ind->data_len = 4 + 8 + len;
        
        qmi_csi_send_ind(lte_mac_svc_client, QMI_TEST_DATA_IND_V01, ind, sizeof(test_data_ind_msg_v01));
    }
    
    a2_ul_phy_write_done_task_fw_org(harq_id, tb_size_in_bits, interrupt_reqd);
}

/* **************** RACH preamble treatment **************** */

/**
 * @brief sends a QMI indication containing the valus used for the RACH preamble
 */
void ul_rach_send_ind() {
    unsigned char* struct_base = &lte_mac_ul_rach_struct;
    unsigned char random_val = *(struct_base + 0x6);
    
    test_data_ind_msg_v01* ind = &indication_buf;
    memset(ind, 0, sizeof(test_data_ind_msg_v01));
    unsigned int* data32 = (unsigned int*)(ind->data);
    *data32 = LTE_MAC_UL_RACH_SVC_ID;
    
    *(ind->data + 4) = random_val;
    *(ind->data + 5) = ul_rach_attempt++; 
    //QMI message fails for size smaller 12!
    ind->data_len = 4 + 8;
    
    qmi_csi_send_ind(lte_mac_svc_client, QMI_TEST_DATA_IND_V01, ind, sizeof(test_data_ind_msg_v01));
}

/**
 * @brief triggers sending of a RACH preamble (first try)
 */
__attribute__ ((overwrite ("lte_mac_rach_send_preamble")))
void lte_mac_rach_send_preamble_hook() {
    lte_mac_rach_send_preamble_fw_org();
        
    if (lte_mac_svc_client != 0) {
        ul_rach_attempt = 0;
        ul_rach_send_ind();
    }
}

/**
 * @brief triggers sending of a RACH preamble (retry)
 */
 __attribute__ ((overwrite ("lte_mac_rach_retry_preamble")))
void lte_mac_rach_retry_preamble_hook() {
    lte_mac_rach_retry_preamble_fw_org();
    
    if (lte_mac_svc_client != 0) {
        ul_rach_send_ind();
    }
}

//TODO frame injection: add new frames, replace frames?, send RACH preamble, disable original modem frames, disable downlink to modem communication (to completly takeover the modem), send at arbitrary times? maybe also changing used blocks in frequency domain in phy layer possible?
