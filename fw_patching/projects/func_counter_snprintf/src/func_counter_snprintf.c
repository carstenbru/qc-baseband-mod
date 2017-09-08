/**
 * @file func_counter_snprintf.c
 * @brief function counters and snprintf example main source file
 *
 * framework capabilities demonstration example, shows:
 * -function overwriting
 * -placement in pointer tables
 * -calling of firmware function
 * -calling of overwritten (original) firmware function
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "func_counter_snprintf.h"

/* function counters */
//memcpy_counter and memset_counter need to be placed in a special section which can always be accessed as they might run with limited memory access rights
unsigned int memcpy_counter __attribute__ ((section (".restricted.data"))) = 0;
unsigned int memset_counter __attribute__ ((section (".restricted.data"))) = 0;
unsigned int snprintf_counter = 0;
unsigned int ping_counter = 0;

/* snprintf client pointer, NULL when no client is set */
void* snprintf_svc_client;

/**
 * @brief QMI ping response function hook
 * 
 * This is a simple example of overwriting a firmware function with own code.
 */
__attribute__ ((overwrite ("qmi_ping_svc_ping_response")))
unsigned int qmi_ping_svc_ping_response_hook (
    void*             clnt_info,
    void*             req_handle,
    unsigned int      msg_id,
    void*             req_c_struct,
    unsigned int      req_c_struct_len,
    void*             service_cookie)
{
    ping_counter++;
    
    test_ping_resp_msg_v01 resp; 
    memset(&resp, 0, sizeof(test_ping_resp_msg_v01));
    memcpy(resp.pong, "hack", 4);
    resp.pong_valid = 1;
    
    qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));
    
    return 0;
}

/**
 * @brief memcpy hook
 * 
 * This demonstrates how to overwrite a function and call the original function, 
 * i.e. extend the original function with own code.
 * Here a call counter is added.
 */
__attribute__ ((overwrite ("memcpy")))
void* memcpy_hook(void* destination, const void* source, unsigned int num) {
    memcpy_counter++;
    return memcpy_fw_org(destination, source, num);
}

/**
 * @brief memset hook
 * 
 * Simliar to memcpy hook, see @ref memcpy_hook
 */
__attribute__ ((overwrite ("memset")))
void* memset_hook(void* ptr, int value, unsigned int num) {
    memset_counter++;
    return memset_fw_org(ptr, value, num);
}

/**
 * @brief generates a response message to a function counter reading request
 */
int func_counter_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    //be careful with alignment when casting! hexagon will produce an exception for unaligned accesses!
    unsigned int* data32 = (unsigned int*)(resp_data);
    *(data32 + 0) = ping_counter;
    *(data32 + 1) = memcpy_counter;
    *(data32 + 2) = memset_counter;
    *(data32 + 3) = snprintf_counter;
    
    return 16;
}

/**
 * @brief snprintf register request handler
 */
int snprintf_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    if (*req_data != 0) {
        snprintf_svc_client = *((void**)clnt_info);
        *((void**)(resp_data)) = snprintf_svc_client;
        return 4;
    } else {
        snprintf_svc_client = 0;
        return 0;
    }
    
    return 0;
}

/**
 * @brief sends an indication with the data printed in an snprintf call to a previously registered client
 */
void send_snprintf_ind(char* src, unsigned int len) {
    if (snprintf_svc_client != 0) {
        if (len > TEST_MED_DATA_SIZE_V01-9) {
            len = TEST_MED_DATA_SIZE_V01-9;
        }
        
        test_data_ind_msg_v01* ind = (test_data_ind_msg_v01*)malloc(sizeof(test_data_ind_msg_v01));
        memset(ind, 0, sizeof(test_data_ind_msg_v01));
        unsigned int* data32 = (unsigned int*)(ind->data);
        *data32 = SNPRINTF_SVC_ID;
        
        //include src pointer to differ snprintf destinations
        *(data32 + 1) = (unsigned int)src;
        ind->data_len = 8 + len + 1;
        memcpy(ind->data + 8, src, len);
        ind->data[8 + len] = 0; //ensure 0 terminated string
        
        qmi_csi_send_ind(snprintf_svc_client, QMI_TEST_DATA_IND_V01, ind, sizeof(test_data_ind_msg_v01));
        
        free(ind);
    }
}

/**
 * @brief snprintf hook, call original function and send QMI indication
 * 
 * This is a quite complex example as snprintf is a variadic function.
 */
__attribute__ ((overwrite ("snprintf")))
int snprintf_hook(char* str, unsigned int size, const char* format, ...) {
    snprintf_counter++;
    
    void *arg = __builtin_apply_args();
    void *ret = __builtin_apply((void*)snprintf_fw_org, arg, 1024);
    
    int written = *((int*)ret); //dirty method to get function result
    send_snprintf_ind(str, written);
    
    __builtin_return(ret);
}
