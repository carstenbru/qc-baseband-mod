/**
 * @file channel_estimation.c
 * @brief channel estimates extraction main source file
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "channel_estimation.h"

/* channel estimation client pointer, NULL when no client is set */
void* ch_est_svc_client;

/* interval in which indications are sent, e.g. 1 means every time, 100 every 100th time */
unsigned short indication_interval;

/* buffer to build indication messages*/
test_data_ind_msg_v01 ind_buf;

__attribute__ ((overwrite ("lte_LL1_log_csf_whitened_matrices")))
void lte_LL1_log_csf_whitened_matrices_hook(unsigned int start_system_sub_frame_number_sfn, unsigned int start_system_frame_number, unsigned int num_whiten_matrices_for_csf, unsigned int num_txant, unsigned int num_rxant, unsigned char* matrices_address) {
    if (ch_est_svc_client != 0) {
        memset(&ind_buf, 0, sizeof(test_data_ind_msg_v01));
        unsigned int* data32 = (unsigned int*)(&ind_buf.data);
        *data32 = CHANNEL_ESTIMATION_SVC_ID;
        
        //include meta-information about reported matrices
        *(ind_buf.data + 4) = (start_system_sub_frame_number_sfn & 0xF) | ((start_system_frame_number & 0xF) << 4);
        *(ind_buf.data + 5) = (start_system_frame_number >> 4);
        *(ind_buf.data + 6) = num_whiten_matrices_for_csf;
        *(ind_buf.data + 7) = ((num_txant & 0xF) << 4) | (num_rxant & 0xF);
        
        unsigned int data_len_bytes = num_whiten_matrices_for_csf * num_txant * num_rxant * 4;
        ind_buf.data_len = 8 + data_len_bytes;
        memcpy(ind_buf.data + 8, matrices_address, data_len_bytes);
        
        qmi_csi_send_ind(ch_est_svc_client, QMI_TEST_DATA_IND_V01, &ind_buf, sizeof(test_data_ind_msg_v01));
    }
    
    //lte_LL1_log_csf_whitened_matrices_fw_org(start_system_sub_frame_number_sfn, start_system_frame_number, num_whiten_matrices_for_csf, num_txant, num_rxant, matrices_address);
}

void set_indication_interval() {
    *((unsigned short*)(lte_LL1_csf_config_pointer + 0xD44)) = indication_interval;
}

__attribute__ ((overwrite ("lte_LL1_csf_callback")))
void lte_LL1_csf_callback_hook(unsigned int a, unsigned int b, unsigned int c) {
    set_indication_interval();
    lte_LL1_csf_callback_fw_org(a, b, c);
}

/**
 * @brief channel estimation register request handler
 */
int channel_estimation_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    if (*req_data != 0) {
        ch_est_svc_client = *((void**)clnt_info);
        *((void**)(resp_data)) = ch_est_svc_client;
        
        indication_interval = *((unsigned short*)req_data);
        set_indication_interval();
        
        return 4; 
    } else {
        ch_est_svc_client = 0;
        return 0;
    }
    
    return 0;
}

