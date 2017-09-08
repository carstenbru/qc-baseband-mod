/**
 * @file qmi_functions_wrapper.h
 * @brief Definitions of existing baseband firmware QMI functions
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __QMI_FUNCTIONS_WRAPPER_H
#define __QMI_FUNCTIONS_WRAPPER_H

/* QMI CSI functions */

ADDRESS(0x08753050) unsigned int qmi_csi_send_resp(void* req_handle, unsigned int msg_id, void* c_struct, unsigned int c_struct_len);
ADDRESS(0x08753100) unsigned int qmi_csi_send_ind(void* client_handle, unsigned int msg_id, void* ind_c_struct, unsigned int c_struct_len);
ADDRESS(0x08752860) unsigned int qmi_csi_register_with_options(void* service_obj, void* service_connect, void* service_disconnect, void* service_process_req, void* service_cookie, void* os_params, void* options, void* service_provider);

/* QMI ping (test) service message handler functions */

ADDRESS(0x08757580) unsigned int qmi_ping_svc_ping_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x08757600) unsigned int qmi_ping_svc_ping_data_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x08757730) unsigned int qmi_ping_svc_ping_large_data_response (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x08757740) unsigned int qmi_ping_svc_ping_data_ind_registration (void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);

ADDRESS(0x09B890C8) unsigned int (*qmi_ping_svc_req_handle_table)(void* clnt_info, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie); 

/* QMI ping (test) service internal functions */

ADDRESS(0x09220424) void* test_get_service_object_v01(void);

ADDRESS(0x087574F0) unsigned int ping_connect_cb(void* client_handle, void* service_cookie, void** connection_handle);
ADDRESS(0x08757530) void ping_disconnect_cb(void* connection_handle, void* service_cookie);
ADDRESS(0x08757540) unsigned int ping_handle_req_cb(void* connection_handle, void* req_handle, unsigned int msg_id, void* req_c_struct, unsigned int req_c_struct_len, void* service_cookie);
ADDRESS(0x08757450) void* qmi_ping_register_service(void* os_params);
 
 
/* QMI ping (test) service definition data structures */
 
ADDRESS(0x0A18BE28) extern void* test_qmi_idl_service_object_v01;

ADDRESS(0x09DF2308) extern void* test_service_command_messages_v01;
ADDRESS(0x09DF2348) extern void* test_service_response_messages_v01;
ADDRESS(0x09DF2388) extern void* test_service_indication_messages_v01;
ADDRESS(0x09DF23A8) extern void* test_qmi_idl_type_table_object_v01;

#endif
