#include FW_WRAPPER

typedef struct {
  int result;
  int error;
} qmi_response_type_v01;

#define TEST_MAX_NAME_SIZE_V01 255

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


/* ######################################## */

__attribute__ ((pointer_table ("qmi_ping_svc_req_handle_table", 0x1E)))
unsigned int ping2_response (
  void*             clnt_info,
  void*             req_handle,
  unsigned int      msg_id,
  void*             req_c_struct,
  unsigned int      req_c_struct_len,
  void*             service_cookie)
{
  test_ping_resp_msg_v01 resp; 
  memset( &resp, 0, sizeof(test_ping_resp_msg_v01) );
  memcpy( resp.pong, "hack", 4 );
  resp.pong_valid = 1;

  qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));      

  return 0;
} 
