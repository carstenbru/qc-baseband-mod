/**
 * @file seemoo_qmi_services.c
 * @brief SEEMOO QMI services on top of test service data requests
 * 
 * These can be used for communication between the SEEMOO QMI Kernel driver and the injected firware code.
 * As a linear search is used to find the right service handler, 
 * this implementation is only suitable for a small amount of services.
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */ 

#include FW_WRAPPER
#include "qmi_message_structs.h"
#include "seemoo_qmi_services.h"

/**
 * @brief QMI ping data request hook, decode message and forward request to our own service handlers
 */
__attribute__ ((pointer_table ("qmi_ping_svc_req_handle_table", 0x21)))
unsigned int services_response_handler (
    void*             clnt_info,
    void*             req_handle,
    unsigned int      msg_id,
    void*             req_c_struct,
    unsigned int      req_c_struct_len,
    void*             service_cookie)
{
    test_data_req_msg_v01* req = (test_data_req_msg_v01 *)req_c_struct;
    unsigned int i;
    
    //decode service ID
    unsigned int svc_id = req->data[0] + (req->data[1] << 8) + (req->data[2] << 16) + (req->data[3] << 24);
    test_data_resp_msg_v01* resp = (test_data_resp_msg_v01*)malloc(sizeof(test_data_resp_msg_v01));
    
    memset(resp, 0, sizeof(test_data_resp_msg_v01));
    //write service ID into response
    resp->data_valid = 1;
    resp->data_len = 4;
    memcpy(resp->data, req->data, 4);
    
    //send request to appropriate service handlers
    for (i = 0; i < seemoo_qmi_services_req_size; i++) {
        if (seemoo_qmi_services_req[i].service_id == svc_id) {
            resp->data_len += (*seemoo_qmi_services_req[i].req_function)(clnt_info, req->data + 4, req->data_len - 4, resp->data + 4);
        }
    }
    
    qmi_csi_send_resp(req_handle, msg_id, resp, sizeof(test_data_resp_msg_v01));
    
    free(resp);
    return 0;
}
