/**
 * @file qmi_message_structs.h
 * @brief header file containing the used QMI data structures
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */ 

#define QMI_TEST_DATA_IND_V01 0x24
#define TEST_MAX_NAME_SIZE_V01 255
#define TEST_MED_DATA_SIZE_V01 8192
#define TEST_DATA_REQ_MAX_MSG_LEN_V01 8456

typedef unsigned char   uint8_t;
typedef unsigned int    uint32_t;

typedef struct {
    int result;
    int error;
} qmi_response_type_v01;

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
    
    /* optional part: service name */
    uint8_t service_name_valid;
    test_name_type_v01 service_name;
} test_ping_resp_msg_v01;

typedef struct {
    /* mandatory part: data */
    uint32_t data_len;
    uint8_t data[TEST_MED_DATA_SIZE_V01];
    
    /* optional part: service name */
    uint8_t client_name_valid;
    test_name_type_v01 client_name;
} test_data_req_msg_v01;

typedef struct {
    /* mandatory part: result code */
    qmi_response_type_v01 resp;
    
    /* optional part: data */
    uint8_t data_valid;
    uint32_t data_len;
    uint8_t data[TEST_MED_DATA_SIZE_V01];
    
    /* optional part: service name */
    uint8_t service_name_valid;
    test_name_type_v01 service_name;
} test_data_resp_msg_v01;

typedef struct {
    /* mandatory part: data */
    uint32_t data_len; 
    uint8_t data[TEST_MED_DATA_SIZE_V01];
    
    /*  optional part: service name */
    uint8_t service_name_valid;
    test_name_type_v01 service_name;
    
    /*  optional part: checksum */
    uint8_t sum_valid;
    uint32_t sum;
} test_data_ind_msg_v01;
