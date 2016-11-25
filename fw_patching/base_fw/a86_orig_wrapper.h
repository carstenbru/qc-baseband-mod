/*
 * fw_wrapper.h
 * 
 * Definitions of original firmware functions
 * 
 * @author Carsten Bruns
 */

#ifndef __FW_WRAPPER_H
#define __FW_WRAPPER_H

/* define address attribute short definition */
#define ADDRESS(x) __attribute__ ((address (x)))

/* ########################################### */

/* firmware identification strings*/

ADDRESS(0x0A0FB088) char* fw_version_string;
ADDRESS(0x0A0FB0A0) char* fw_time_string;
ADDRESS(0x0A0FB0D0) char* fw_time_string2;
ADDRESS(0x0A0FB0B0) char* fw_date_string;
ADDRESS(0x0A0FB0E0) char* fw_date_string2;

/* standard C functions */

typedef unsigned int size_t;

ADDRESS(0x092B9AB0) int printf(const char * format, ...);
ADDRESS(0x09130360) void* memcpy(void* destination, const void* source, size_t num);
ADDRESS(0x09130650) void* memset(void* ptr, int value, size_t num);

/* QMI functions */

ADDRESS(0x097A9E80) unsigned int qmi_csi_send_resp(void* req_handle, unsigned int msg_id, void* c_struct, unsigned int c_struct_len);

ADDRESS(0x097AF0B0) unsigned int qmi_ping_svc_ping_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x097AF2D0) unsigned int qmi_ping_svc_ping_data_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x097AF220) unsigned int qmi_ping_svc_ping_large_data_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x097AF130) unsigned int qmi_ping_svc_ping_data_ind_registration (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);

/*
real function declaration data types:
 qmi_csi_cb_error (* const req_handle_table) (
    client_info_type         *clnt_info,
    qmi_req_handle           req_handle,
    unsigned int             msg_id,
    void                     *req_c_struct,
    unsigned int             req_c_struct_len,
    void                     *service_cookie);
 */
ADDRESS(0x0A0FF620) unsigned int (*qmi_ping_svc_req_handle_table)(void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);

/* other */

#endif /* __FW_WRAPPER_H */ 
