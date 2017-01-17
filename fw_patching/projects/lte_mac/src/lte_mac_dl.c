/**
 * @file lte_mac_dl.c
 * @brief LTE MAC Layer messages forwarding (over QMI) patches - downlink and service registration part
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "lte_mac.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

/**
 * @brief downlink transport block information structure
 */
typedef struct {
  unsigned char* tb_dsm_ptr; /*!< Pointer to the dsm chain containing the transport block. */
  
  uint16 tb_len;      /*!< The length in bytes of the transport block */
  uint32 word1; /*!< The meta-information contained in the transport block. */
  uint32 word2; 
} a2_dl_phy_transport_block_t; 

/* LTE MAC client pointer, NULL when no client is set */
void* lte_mac_svc_client;

/* structure used to build QMI indications */
test_data_ind_msg_v01 indication_buf;

/**
 * @brief sends an QMI indication containing a LTE MAC downlink frame
 * 
 * @param transport_block received downlink transport block
 */
void send_lte_mac_ind(a2_dl_phy_transport_block_t* transport_block) {
    test_data_ind_msg_v01* ind = &indication_buf;
    memset(ind, 0, sizeof(test_data_ind_msg_v01));
    unsigned int* data32 = (unsigned int*)(ind->data);
    *data32 = LTE_MAC_DL_SVC_ID;
    
    //include metadata
    *(data32 + 1) = transport_block->word1;
    *(data32 + 2) = transport_block->word2;
    
    //copy actual data, limit size
    unsigned int len = transport_block->tb_len;
    if (len > TEST_MED_DATA_SIZE_V01-12) {
        len = TEST_MED_DATA_SIZE_V01-12;
    }
    ind->data_len = 12 + len ;
    dsm_extract(transport_block->tb_dsm_ptr, 0, ind->data + 12, TEST_MED_DATA_SIZE_V01-12);
    
    qmi_csi_send_ind(lte_mac_svc_client, QMI_TEST_DATA_IND_V01, ind, sizeof(test_data_ind_msg_v01));    
}

/**
 * @brief hook of the transport block receive function in the LTE MAC downlink
 */
__attribute__ ((overwrite ("lte_mac_dl_process_transport_block")))
void lte_mac_dl_process_transport_block_hook(a2_dl_phy_transport_block_t* transport_block) {
    if (lte_mac_svc_client != 0) {
        send_lte_mac_ind(transport_block);
    }
    return lte_mac_dl_process_transport_block_fw_org(transport_block);
}

/**
 * @brief lte_mac register request handler
 */
int lte_mac_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    if (*req_data != 0) {
        lte_mac_svc_client = *((void**)clnt_info);
        *((void**)(resp_data)) = lte_mac_svc_client;
        return 4;
    } else {
        lte_mac_svc_client = 0;
        return 0;
    }
    
    return 0;
}
