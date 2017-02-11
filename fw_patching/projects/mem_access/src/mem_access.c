/**
 * @file mem_access.c
 * @brief memory access (read/write) example main source file
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "mem_access.h"

unsigned int read_length(unsigned int* data) {
    unsigned int length = *data;
    
    //limit length to QMI packet data size
    if (length > TEST_MED_DATA_SIZE_V01 - 8 - 4) {
        length = TEST_MED_DATA_SIZE_V01 - 8 - 4;
    }
    
    return length;
}

/**
 * @brief generates a response message to a memory reading request
 */
int mem_access_read_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    unsigned int* req_data32 = (unsigned int*)(req_data);
    unsigned int start_address = *(req_data32 + 0);
    unsigned int length = read_length(req_data32 + 1);
    
    //include request data to verify and in case many requests were send at the same time
    unsigned int* data32 = (unsigned int*)(resp_data);
    *(data32 + 0) = start_address;
    *(data32 + 1) = length;
    
    //copy requested data
    memcpy(resp_data + 8, (unsigned char*)start_address, length);
    
    return 8 + length;
}

/**
 * @brief memory write request handler
 */
int mem_access_write_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    unsigned int* req_data32 = (unsigned int*)(req_data);
    unsigned int start_address = *(req_data32 + 0);
    unsigned int length = read_length(req_data32 + 1);
    
    unsigned int* data32 = (unsigned int*)(resp_data);
    *(data32 + 0) = start_address;
    *(data32 + 1) = length;
    
    //copy received data
    memcpy((unsigned char*)start_address, req_data + 8, length);
    
    return 8;
}
