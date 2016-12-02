/**
 * @file test_handler.c
 * @brief simple framework "pointer_table" and "overwrite" attribute test, overwriting QMI test service ping-handler, introducing a new handler and demonstrating a call to an original FW function which was overwritten
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER

#define SEEMOO_PING_REQ_MSG_ID 0x1E

typedef struct {
  int result;
  int error;
} qmi_response_type_v01;

#define TEST_MAX_NAME_SIZE_V01 255

#define TEST_MED_DATA_SIZE_V01 8192
#define TEST_DATA_REQ_MAX_MSG_LEN_V01 8456

typedef unsigned char   uint8_t;
typedef unsigned int    uint32_t;

typedef struct {
  uint32_t name_len;
  char name[TEST_MAX_NAME_SIZE_V01];
} test_name_type_v01;
 
typedef struct {
  /* mandatory part: result code */
  qmi_response_type_v01 resp;

  /* optional part: pong */
  uint8_t pong_valid;
  char pong[4];

  /*  optional part: service name */ //TODO remove that
  uint8_t service_name_valid;
  test_name_type_v01 service_name;
} test_ping_resp_msg_v01;

typedef struct {
	uint32_t data_len;
	uint8_t data[TEST_MED_DATA_SIZE_V01];

	uint8_t client_name_valid;
	test_name_type_v01 client_name;
} test_data_req_msg_v01;

typedef struct {
	qmi_response_type_v01 resp;

	uint8_t data_valid;
	uint32_t data_len;
	uint8_t data[TEST_MED_DATA_SIZE_V01];

	uint8_t service_name_valid;
	test_name_type_v01 service_name;
} test_data_resp_msg_v01;

/* ######################################## */

unsigned int ping_counter = 0;

/*
__attribute__ ((overwrite ("memcpy")))
void* memcpy_hook(void* destination, const void* source, unsigned int num) {
    //memcpy_counter++;
    return memcpy_fw_org(destination, source, num);
}*/

/*
__attribute__ ((overwrite ("memset")))
void* memset_hook(void* ptr, int value, unsigned int num) {
    memset_counter++;
    return memset_fw_org(ptr, value, num);
}*/

__attribute__ ((overwrite ("qmi_ping_svc_ping_response")))
unsigned int qmi_ping_svc_ping_response_hook (
  void*             clnt_info,
  void*             req_handle,
  unsigned int      msg_id,
  void*             req_c_struct,
  unsigned int      req_c_struct_len,
  void*             service_cookie)
{
//   test_ping_resp_msg_v01 resp; 
//   memset(&resp, 0, sizeof(test_ping_resp_msg_v01) );
//   memcpy(resp.pong, "ponk", 4 );
//   resp.pong_valid = 1;
// 
//   qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));    
// 
//   return 0;
    
    ping_counter++;
    return qmi_ping_svc_ping_response_fw_org(clnt_info, req_handle, msg_id, req_c_struct, req_c_struct_len, service_cookie);
}

__attribute__ ((pointer_table ("qmi_ping_svc_req_handle_table", 0x21)))
unsigned int seemoo_data_response (
  void*             clnt_info,
  void*             req_handle,
  unsigned int      msg_id,
  void*             req_c_struct,
  unsigned int      req_c_struct_len,
  void*             service_cookie)
{
  test_data_req_msg_v01 *req = (test_data_req_msg_v01 *)req_c_struct;
  test_data_resp_msg_v01 *resp;

  resp = (test_data_resp_msg_v01*)malloc(sizeof(test_data_resp_msg_v01));

  memset(resp, 0, sizeof(test_data_resp_msg_v01) );
  resp->data_len = req->data_len-4;
  resp->data_valid = 1;
  if(resp->data_len > 0 && resp->data_len <= TEST_MED_DATA_SIZE_V01 ) {
    memcpy(resp->data, req->data, resp->data_len);
  }
  
  resp->data[0] = ping_counter;

  qmi_csi_send_resp(req_handle, msg_id, resp, sizeof(test_data_resp_msg_v01) );

  free(resp);

  return 0;
} 
