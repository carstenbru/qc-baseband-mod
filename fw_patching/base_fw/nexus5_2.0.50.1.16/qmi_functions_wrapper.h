/**
 * @file qmi_functions_wrapper.h
 * @brief Definitions of existing baseband firmware QMI functions
 * 
 * Specific to Asus Padfone Infinity 2 (A86), firmware version "M3.12.15-A86_1032"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __QMI_FUNCTIONS_WRAPPER_H
#define __QMI_FUNCTIONS_WRAPPER_H

/* QMI CSI functions */

ADDRESS("unknown" /* candidates: 0x094C50B0, 0x094C5310 */) unsigned int qmi_csi_send_resp(void* req_handle, unsigned int msg_id, void* c_struct, unsigned int c_struct_len);
ADDRESS(0x094C51B0) unsigned int qmi_csi_send_ind(void* client_handle, unsigned int msg_id, void* ind_c_struct, unsigned int c_struct_len);
ADDRESS(0x094C4390) unsigned int qmi_csi_register_with_options(void* service_obj, void* service_connect, void* service_disconnect, void* service_process_req, void* service_cookie, void* os_params, void* options, void* service_provider);

/* QMI ping (test) service message handler functions */

ADDRESS(0x094CA7A0) unsigned int qmi_ping_svc_ping_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x094CAB80) unsigned int qmi_ping_svc_ping_data_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x094CAAA0) unsigned int qmi_ping_svc_ping_large_data_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x08FD5FBC) unsigned int qmi_ping_svc_ping_data_ind_registration (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);

ADDRESS("unknown") unsigned int (*qmi_ping_svc_req_handle_table)(void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie); 

/* QMI ping (test) service internal functions */

ADDRESS(0x09C46030) void* test_get_service_object_v01(void);

ADDRESS("unknown" /* candidates: 0x09504250, 0x09508C10, 0x0950DCA0, 0x09F58030 */) unsigned int ping_connect_cb(void* client_handle, void* service_cookie, void** connection_handle);
ADDRESS("unknown") void ping_disconnect_cb(void* connection_handle, void* service_cookie);
ADDRESS(0x094CA6B0) unsigned int ping_handle_req_cb(void* connection_handle, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x094CA6E0) void* qmi_ping_register_service(void* os_params);
 
 
/* QMI ping (test) service definition data structures */
 
ADDRESS("unknown") extern void* test_qmi_idl_service_object_v01;

ADDRESS("unknown") extern void* test_service_command_messages_v01;
ADDRESS("unknown") extern void* test_service_response_messages_v01;
ADDRESS("unknown") extern void* test_service_indication_messages_v01;
ADDRESS("unknown") extern void* test_qmi_idl_type_table_object_v01;

#endif