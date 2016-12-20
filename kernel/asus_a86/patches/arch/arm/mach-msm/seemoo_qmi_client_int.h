/** 
 * @file seemoo_qmi_client_int.h
 * @brief SEEMOO QMI client module header for internally used definitions
 * 
 * This is the kernel client implementation for the functionality in the firmware mod examples.
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 * 
 * #########################
 * 
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define TEST_SERVICE_SVC_ID 0x0000000f
#define TEST_SERVICE_INS_ID 1

#define QMI_TEST_DATA_IND_V01 0x24
#define TEST_DATA_IND_MAX_MSG_LEN_V01 8461
#define DATA_IND1_TLV_TYPE 0x1
#define DATA_OPT1_TLV_TYPE 0x10
#define DATA_OPT2_TLV_TYPE 0x11

#define WRITE_BUF_SIZE 65536
#define HANDLE_ID_NORM 0
#define HANDLE_ID_IND 1

struct seemoo_qmi_packet_struct_t {
    unsigned int length;
    unsigned char data[TEST_MED_DATA_SIZE_V01];
    struct seemoo_qmi_packet_struct_t* next;
};

typedef struct seemoo_qmi_packet_struct_t seemoo_qmi_packet_t;

/* ############################### */

typedef struct {
	/* mandatory part: data */
	uint32_t data_len; 
	uint8_t data[TEST_MED_DATA_SIZE_V01];
	
	/*  optional part: service name */
	uint8_t service_name_valid;
	struct test_name_type_v01 service_name;
	
	/*  optional part: checksum */
	uint8_t sum_valid;
	uint32_t sum;
} test_data_ind_msg_v01;

static struct elem_info test_name_type_v01_ei[] = {
	{
		.data_type      = QMI_DATA_LEN,
		.elem_len	= 1,
		.elem_size      = sizeof(uint8_t),
		.is_array	= NO_ARRAY,
		.tlv_type	= QMI_COMMON_TLV_TYPE,
		.offset		= offsetof(struct test_name_type_v01,
							   name_len),
	},
	{
		.data_type      = QMI_UNSIGNED_1_BYTE,
		.elem_len       = TEST_MAX_NAME_SIZE_V01,
		.elem_size      = sizeof(char),
		.is_array       = VAR_LEN_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
		.offset         = offsetof(struct test_name_type_v01,
								   name),
	},
	{
		.data_type      = QMI_EOTI,
		.is_array       = NO_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
	},
};

struct elem_info test_data_ind_msg_v01_ei[] = {
	{
		.data_type      = QMI_DATA_LEN,
		.elem_len       = 1,
		.elem_size      = sizeof(uint32_t),
		.is_array       = NO_ARRAY,
		.tlv_type       = DATA_IND1_TLV_TYPE,
		.offset         = offsetof(test_data_ind_msg_v01,
								   data_len),
	},
	{
		.data_type      = QMI_UNSIGNED_1_BYTE,
		.elem_len       = TEST_MED_DATA_SIZE_V01,
		.elem_size      = sizeof(uint8_t),
		.is_array       = VAR_LEN_ARRAY,
		.tlv_type       = DATA_IND1_TLV_TYPE,
		.offset         = offsetof(test_data_ind_msg_v01,
								   data),
	},
	{
		.data_type      = QMI_OPT_FLAG,
		.elem_len       = 1,
		.elem_size      = sizeof(uint8_t),
		.is_array       = NO_ARRAY,
		.tlv_type       = DATA_OPT1_TLV_TYPE,
		.offset         = offsetof(test_data_ind_msg_v01,
								   service_name_valid),
	},
	{
		.data_type      = QMI_STRUCT,
		.elem_len       = 1,
		.elem_size      = sizeof(struct test_name_type_v01),
		.is_array       = NO_ARRAY,
		.tlv_type       = DATA_OPT1_TLV_TYPE,
		.offset         = offsetof(test_data_ind_msg_v01,
								   service_name),
								   .ei_array       = test_name_type_v01_ei,
	},
	{
		.data_type      = QMI_OPT_FLAG,
		.elem_len       = 1,
		.elem_size      = sizeof(uint8_t),
		.is_array       = NO_ARRAY,
		.tlv_type       = DATA_OPT2_TLV_TYPE,
		.offset         = offsetof(test_data_ind_msg_v01,
								   sum_valid),
	},
	{
		.data_type      = QMI_DATA_LEN,
		.elem_len       = 1,
		.elem_size      = sizeof(uint32_t),
		.is_array       = NO_ARRAY,
		.tlv_type       = DATA_OPT2_TLV_TYPE,
		.offset         = offsetof(test_data_ind_msg_v01,
								   sum),
	},
	{
		.data_type      = QMI_EOTI,
		.is_array       = NO_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
	},
};
