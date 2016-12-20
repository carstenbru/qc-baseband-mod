/**
 * @file seemoo_qmi_services.h
 * @brief SEEMOO QMI services on top of test service data requests
 * 
 * These can be used for communication between the SEEMOO QMI Kernel driver and the injected firware code.
 * As a linear search is used to find the right service handler, 
 * this implementation is only suitable for a small amount of services.
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */ 

#ifndef __SEEMOO_QMI_SERVICES_H
#define __SEEMOO_QMI_SERVICES_H

/**
 * @brief function pointer type for handler functions for incoming QMI request
 * 
 * @param clnt_info structure containing information about the sending QMI client
 * @param req_data pointer to (user payload) data in request
 * @param req_data_len number of bytes in req_data
 * @param resp_data pointer where bytes for the response should be places
 * @return number of response bytes written
 */
typedef int (*seemoo_qmi_services_req_func_t)(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

/**
 * @brief struct defining a SEEMOO QMI service handler
 */
typedef struct {
    unsigned int service_id; /**< Service ID (32bit, highest bit has to be 0  */
    seemoo_qmi_services_req_func_t req_function; /**< request handler function @see seemoo_qmi_services_req_func_t */
} seemoo_qmi_services_req_t;

/**
 * @brief array of known SEEMOO QMI services, has to be defined somewhere in the source code!
 */
extern seemoo_qmi_services_req_t seemoo_qmi_services_req[];
/**
 * @brief number of known SEEMOO QMI services, has to be defined somewhere in the source code!
 */
extern int seemoo_qmi_services_req_size;

#endif
