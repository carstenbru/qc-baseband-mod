/** 
 * @file seemoo_qmi_client.c
 * @brief SEEMOO QMI client module
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

#include <stdarg.h>

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/qmi_encdec.h>

#include <asm/uaccess.h>

#include <mach/msm_qmi_interface.h>

#include "kernel_test_service_v01.h"

#include "seemoo_qmi_client_int.h"

/* Variable to initiate the test through debugfs interface */
static struct dentry *dent;

/* 
 * client ports for IPC Router 
 * remark: we use two handles as the QMI kernel code crashes 
 * when indications arrive interleaved with normal message responses
 */
static struct qmi_handle* seemoo_clnt;
static int seemoo_clnt_reset;
static struct qmi_handle* seemoo_clnt_ind;
static int seemoo_clnt_ind_reset;

/* Reader thread to receive responses & indications */
static void recv_msg(struct work_struct *work);
static DECLARE_DELAYED_WORK(work_recv_msg, recv_msg);
static void recv_msg_ind(struct work_struct *work);
static DECLARE_DELAYED_WORK(work_recv_msg_ind, recv_msg_ind);
static void svc_arrive(struct work_struct *work);
static DECLARE_DELAYED_WORK(work_svc_arrive, svc_arrive);
static void svc_exit(struct work_struct *work);
static DECLARE_DELAYED_WORK(work_svc_exit, svc_exit);
static struct workqueue_struct *workqueue;
static int work_recv_msg_count, work_recv_msg_ind_count;

/* write buffers for debugfs, will be returned on next read */
char write_buf[WRITE_BUF_SIZE];
int write_pos;

char snprintf_buf[WRITE_BUF_SIZE];
int snprintf_pos;

/* ############################### */

/**
 * @brief writes a text into the write buffer, call like printf
 */
static void write_text(char* format, ...) {
	va_list args;
	va_start(args, format);
	
	write_pos += vsnprintf(write_buf+write_pos, sizeof(write_buf)-write_pos, format, args);
	
	va_end(args);
}

/**
 * @brief writes a text into the error buffer, call like printf
 */
static void write_error(char* format, ...) {
	va_list args;
	va_start(args, format);
	
	write_pos += vsnprintf(write_buf+write_pos, sizeof(write_buf)-write_pos, format, args);
	
	va_end(args);
}

/**
 * @brief writes a text into the "snprintf" (from modem) buffer, call like printf
 */
static void write_snprintf(char* format, ...) {
	va_list args;
	va_start(args, format);
	
	snprintf_pos += vsnprintf(snprintf_buf+snprintf_pos, sizeof(snprintf_buf)-snprintf_pos, format, args);
	
	va_end(args);
}

/**
 * @brief sends a "ping" message to the modem and prints the result
 */
static int ping_pong_send_sync_msg(void)
{
	struct test_ping_req_msg_v01 req;
	struct test_ping_resp_msg_v01 resp;
	struct msg_desc req_desc, resp_desc;
	int rc;
	
	memcpy(req.ping, "ping", sizeof(req.ping));
	req.client_name_valid = 0;
	
	req_desc.max_msg_len = TEST_PING_REQ_MAX_MSG_LEN_V01;
	req_desc.msg_id = TEST_PING_REQ_MSG_ID_V01;
	req_desc.ei_array = test_ping_req_msg_v01_ei;
	
	resp_desc.max_msg_len = TEST_PING_REQ_MAX_MSG_LEN_V01;
	resp_desc.msg_id = TEST_PING_REQ_MSG_ID_V01;
	resp_desc.ei_array = test_ping_resp_msg_v01_ei;
	
	rc = qmi_send_req_wait(seemoo_clnt, &req_desc, &req, sizeof(req),
						   &resp_desc, &resp, sizeof(resp), 1000);
	
	if (rc < 0) {
		write_error("%s: send req failed %d\n", __func__, rc);
		return rc;
	}
	
	write_text("%s: Received %.4s response\n", __func__, resp.pong);
	return rc;
}

/**
 * @brief handler for responses of unknown services, prints the messages data payload raw bytes
 * 
 * @param resp_svc_id service ID indicated in the message
 * @param resp response message
 */
static void unknown_svc_handler(unsigned int resp_svc_id, struct test_data_resp_msg_v01* resp) {
	int i, f;
	unsigned int d;
	
	write_text("unknown service ID 0x%08X, printing raw data from response\n", resp_svc_id);
	write_text("data_valid %d\n", resp->data_valid);
	write_text("data_len %d\n", resp->data_len);
	for (i = 0; i+3 < resp->data_len; i+=4) {
		write_text("0x%02X|", i);
		d = 0;
		for (f = 0; f < 4; f++) {
			write_text("\t0x%02X", resp->data[i + f]);
			d += resp->data[i + f] << (8*f);
		}
		write_text("\t| %d\n", d);
	}
	write_text("\n");
}

/**
 * @brief read integer as little endian
 * 
 * @param data array of bytes containing the little endian integer
 * @return resulting (unsigned) integer value
 */
static unsigned int read_int_little_endian(char* data) {
	return *data + (*(data + 1) << 8) + (*(data + 2) << 16) + (*(data + 3) << 24);
}

/**
 * @brief handler for responses of function counter service
 * 
 * @param resp response message
 */
static void func_counter_svc_handler(struct test_data_resp_msg_v01* resp) {
	write_text("function counter service:\n\n");
	write_text("qmi_ping_svc_ping_response:\t%u\n", read_int_little_endian(resp->data+4));
	write_text("memcpy:\t\t%u\n", read_int_little_endian(resp->data+8));
	write_text("memset:\t\t%u\n", read_int_little_endian(resp->data+12));
	write_text("snprintf:\t%u\n", read_int_little_endian(resp->data+16));
}

/**
 * @brief handler for responses of snprintf (registration) service
 * 
 * @param resp response message
 */
static void snprintf_svc_handler(struct test_data_resp_msg_v01* resp) {
	if (resp->data_len == 8) {
		write_text("snprintf service success\n\n");
	}
}

/**
 * @brief sends a synchronous (i.e. waits for the result) message to a service
 *
 * @param handle client handle to use
 * @param svc destination service ID
 * @param data_len payload data length
 * @param data payload data as array of bytes
 */
static int services_send_sync_msg(struct qmi_handle* handle, unsigned int svc, unsigned int data_len, unsigned char* data) {
	struct test_data_req_msg_v01* req;
	struct test_data_resp_msg_v01* resp;
	struct msg_desc req_desc, resp_desc;
	int rc;
	unsigned int resp_svc_id;
	
	/* allocate memory for request and response */
	req = kzalloc(sizeof(struct test_data_req_msg_v01), GFP_KERNEL);
	if (!req) {
		write_error("%s: Data req msg alloc failed\n", __func__);
		return -ENOMEM;
	}
	
	resp = kzalloc(sizeof(struct test_data_resp_msg_v01), GFP_KERNEL);
	if (!resp) {
		write_error("%s: Data resp msg alloc failed\n", __func__);
		kfree(req);
		return -ENOMEM;
	}
	
	/* include 322bit service ID as first 4 bytes of data */
	req->data_len = 4 + data_len;
	req->data[0] = svc & 0xFF;
	req->data[1] = (svc >> 8) & 0xFF;
	req->data[2] = (svc >> 16) & 0xFF;
	req->data[3] = (svc >> 24) & 0xFF;
	if (data_len) { //include payload data (if passed)
		memcpy(req->data + 4, data, data_len);
	}
	
	req_desc.max_msg_len = TEST_DATA_REQ_MAX_MSG_LEN_V01;
	req_desc.msg_id = TEST_DATA_REQ_MSG_ID_V01;
	req_desc.ei_array = test_data_req_msg_v01_ei;
	
	resp_desc.max_msg_len = TEST_DATA_REQ_MAX_MSG_LEN_V01;
	resp_desc.msg_id = TEST_DATA_REQ_MSG_ID_V01;
	resp_desc.ei_array = test_data_resp_msg_v01_ei;
	
	//send request and wait for reply
	rc = qmi_send_req_wait(handle, &req_desc, req, sizeof(*req),
						   &resp_desc, resp, sizeof(*resp), 1000);
	if (rc < 0) {
		write_error("%s: send req failed\n", __func__);
		goto data_send_err;
	}
	
	if (!resp->data_valid) {
		goto data_send_err;
	}
	
	//read response service ID (should be identical to send one but to check)
	resp_svc_id = read_int_little_endian(resp->data);
	
	/* dispatch response to right handler function */
	switch (resp_svc_id) {
		case (FUNC_COUNTER_SVC_ID):
			func_counter_svc_handler(resp);
			break;
		case (SNPRINTF_SVC_ID):
			snprintf_svc_handler(resp);
			break;
		default:
			unknown_svc_handler(resp_svc_id, resp);
			break;
	}
data_send_err:
	kfree(resp);
	kfree(req);
	return rc;
}

/**
 * @brief incoming message handler
 */
static void recv_msg(struct work_struct *work) {
	int rc;
	
	//receive all arrived messages (this handler is only called once)
	do {
		rc = qmi_recv_msg(seemoo_clnt);
		if (rc < 0)
			write_error("%s: Error receiving message\n", __func__);
	} while (--work_recv_msg_count);
}

/**
 * @brief incoming message handler for indication handle
 */
static void recv_msg_ind(struct work_struct *work) {
	int rc;
	
	//receive all arrived messages (this handler is only called once)
	do {
		rc = qmi_recv_msg(seemoo_clnt_ind);
		if (rc < 0)
			write_error("%s: Error receiving message\n", __func__);
	} while (--work_recv_msg_ind_count);
}

/**
 * @brief QMI client notification handler
 * 
 * counta messages as items are only added once to the workqueue
 */
static void clnt_notify(struct qmi_handle *handle,
						enum qmi_event_type event, void *notify_priv)
{
	int handle_id = (int)notify_priv;
	switch (event) {
		case QMI_RECV_MSG:
			// check for which handler this notification is
			if (handle_id == HANDLE_ID_NORM) {
				work_recv_msg_count++;
				queue_delayed_work(workqueue,
							   &work_recv_msg, 0);
			} else if (handle_id == HANDLE_ID_IND) {
				work_recv_msg_ind_count++;
				queue_delayed_work(workqueue,
							   &work_recv_msg_ind, 0);
			}
			break;
		default:
			break;
	}
}

/**
 * @brief snprintf indication receive callback function
 * 
 * @param handle origin handle
 * @param msg_id message service ID
 * @param msg pointer to undecoded (!) message
 * @param msg_len length of the message in bytes
 */
static void snprintf_ind_cb(struct qmi_handle* handle, unsigned int msg_id, void* msg, unsigned int msg_len, void* ind_cb_priv) {
	struct msg_desc ind_desc;
	test_data_ind_msg_v01* ind;
	unsigned int svc_id;
	int rc;
	
	// check if the message originates from the snprintf server
	if (msg_id ==  QMI_TEST_DATA_IND_V01) {
		/* decode the message by using the descriptor */
		ind_desc.max_msg_len = TEST_DATA_IND_MAX_MSG_LEN_V01;
		ind_desc.msg_id = QMI_TEST_DATA_IND_V01;
		ind_desc.ei_array = test_data_ind_msg_v01_ei;
		
		ind = (test_data_ind_msg_v01*)kzalloc(sizeof(test_data_ind_msg_v01), GFP_KERNEL);
		if (ind == 0) {
			write_error("%s: could not allocate memory for indication struct", __func__);
			return;
		}
		rc = qmi_kernel_decode(&ind_desc, ind, (void*)(((char*)msg) + QMI_HEADER_SIZE), msg_len-QMI_HEADER_SIZE);
		/* check for errors */
		if (rc != 0) {
			write_error("%s: decode failure", __func__);
			return;
		}
		if (ind->data_len < 9) {
			write_error("%s: received too short message", __func__);
			return;
		}
		
		//check service ID in message data field
		svc_id = read_int_little_endian(ind->data);
		if (svc_id == SNPRINTF_SVC_ID) {
			//decode snprintf write destination pointer
			unsigned int write_dest = read_int_little_endian(ind->data + 4);
			*(ind->data + ind->data_len - 1) = 0; //ensure 0 terminated string
			write_snprintf("%u:\t%s\n", write_dest, ind->data + 8); //output message
		} else {
			write_error("%s: unknown service ID 0x%08X\n", __func__, svc_id);
		}
		kfree(ind);
	}
}

/**
 * @brief service arrive handler
 */
static void svc_arrive(struct work_struct *work)
{
	int rc;
	
	write_text("service arrived!\n");
	
	/* create local client ports for QMI communication */
	seemoo_clnt = qmi_handle_create(clnt_notify, (void*)HANDLE_ID_NORM);
	seemoo_clnt_ind = qmi_handle_create(clnt_notify, (void*)HANDLE_ID_IND);
	if (!seemoo_clnt || !seemoo_clnt_ind) {
		write_error("%s: QMI client handle alloc failed\n", __func__);
		return;
	}
	
	/* connect to service, check for errors */
	rc = qmi_connect_to_service(seemoo_clnt, TEST_SERVICE_SVC_ID,
								TEST_SERVICE_INS_ID);
	qmi_connect_to_service(seemoo_clnt_ind, TEST_SERVICE_SVC_ID,
								TEST_SERVICE_INS_ID);
	if (rc < 0) {
		write_error("%s: Server not found\n", __func__);
		qmi_handle_destroy(seemoo_clnt);
		seemoo_clnt = NULL;
		return;
	}
	
	/* register callback function for indications */
	rc = qmi_register_ind_cb(seemoo_clnt_ind, snprintf_ind_cb, NULL);
	if (rc < 0) {
		write_error("%s: could not register indication callback\n", __func__);
		return;
	}
	
	seemoo_clnt_reset = 0;
	seemoo_clnt_ind_reset = 0;
}

/**
 * @brief service leave handler
 */
static void svc_exit(struct work_struct *work)
{
	write_text("service left\n");
	
	qmi_handle_destroy(seemoo_clnt);
	seemoo_clnt_reset = 1;
	seemoo_clnt = NULL;
	qmi_handle_destroy(seemoo_clnt_ind);
	seemoo_clnt_ind_reset = 1;
	seemoo_clnt_ind = NULL;
}

/**
 * @brief service event handler
 */
static int svc_event_notify(struct notifier_block *this,
							unsigned long code,
							void *_cmd)
{
	switch (code) {
		case QMI_SERVER_ARRIVE:
			queue_delayed_work(workqueue,
							   &work_svc_arrive, 0);
			break;
		case QMI_SERVER_EXIT:
			queue_delayed_work(workqueue,
							   &work_svc_exit, 0);
			break;
		default:
			break;
	}
	return 0;
}

/* TODO: these will be replaced by another user interface later */

static int debugfs_open(struct inode *ip, struct file *fp)
{
	if (!seemoo_clnt) {
		write_error("%s SEEMOO client is not initialized\n", __func__);
		return -ENODEV;
	}
	return 0;
}

static ssize_t debugfs_read(struct file *fp, char __user *buf,
							size_t count, loff_t *pos)
{
	ssize_t s = simple_read_from_buffer(buf, count, pos,
										write_buf, strnlen(write_buf, WRITE_BUF_SIZE));
	write_buf[0] = 0;
	write_pos = 0;
	return s;
}

static ssize_t debugfs_snprintf(struct file *fp, char __user *buf,
								size_t count, loff_t *pos)
{
	ssize_t s = simple_read_from_buffer(buf, count, pos,
										snprintf_buf, strnlen(snprintf_buf, WRITE_BUF_SIZE));
	snprintf_buf[0] = 0;
	snprintf_pos = 0;
	return s;
}

static ssize_t debugfs_func_counter_read(struct file *fp, char __user *buf,
										 size_t count, loff_t *pos)
{
	ssize_t s;
	int write_pos_tmp = write_pos;
	
	//send message to read function counters
	services_send_sync_msg(seemoo_clnt, FUNC_COUNTER_SVC_ID, 4, "dummy");
	s = simple_read_from_buffer(buf, count, pos,
								write_buf+write_pos_tmp, strnlen(write_buf+write_pos_tmp, WRITE_BUF_SIZE));
	write_buf[write_pos_tmp] = 0;
	write_pos = write_pos_tmp;
	return s;
}

static int debugfs_release(struct inode *ip, struct file *fp)
{
	return 0;
}

static ssize_t debugfs_write(struct file *fp, const char __user *buf,
							 size_t count, loff_t *pos)
{
	unsigned char cmd[64];
	int len;
	int last_res;
	
	if (count < 1)
		return 0;
	
	len = min(count, (sizeof(cmd) - 1));
	
	if (copy_from_user(cmd, buf, len))
		return -EFAULT;
	
	cmd[len] = 0;
	if (cmd[len-1] == '\n') {
		cmd[len-1] = 0;
		len--;
	}
	
	last_res = 0;
	/* check user input and send corresponding command to modem */
	if (!strncmp(cmd, "pp", sizeof(cmd))) {
		last_res = ping_pong_send_sync_msg();
	} else if (!strncmp(cmd, "data", sizeof(cmd))) {
		last_res = services_send_sync_msg(seemoo_clnt, FUNC_COUNTER_SVC_ID, 0, NULL);
	} else if (!strncmp(cmd, "print0", sizeof(cmd))) {
		char reg = 0;
		last_res = services_send_sync_msg(seemoo_clnt, SNPRINTF_SVC_ID, 1, &reg);
	} else if (!strncmp(cmd, "print1", sizeof(cmd))) {
		char reg = 1;
		last_res = services_send_sync_msg(seemoo_clnt_ind, SNPRINTF_SVC_ID, 1, &reg);
	} else {
		last_res = -EINVAL;
	}
	
	if (last_res == -ENETRESET || seemoo_clnt_reset || seemoo_clnt_ind_reset) {
		do {
			msleep(50);
		} while (seemoo_clnt_reset || seemoo_clnt_ind_reset);
	}
	
	return count;
}

static struct notifier_block seemoo_clnt_nb = {
	.notifier_call = svc_event_notify,
};

static const struct file_operations debug_ops = {
	.owner = THIS_MODULE,
	.open = debugfs_open,
	.read = debugfs_read,
	.write = debugfs_write,
	.release = debugfs_release,
};

static const struct file_operations debug_func_counter_ops = {
	.owner = THIS_MODULE,
	.open = debugfs_open,
	.read = debugfs_func_counter_read,
	.write = debugfs_write,
	.release = debugfs_release,
};

static const struct file_operations debug_snprintf_ops = {
	.owner = THIS_MODULE,
	.open = debugfs_open,
	.read = debugfs_snprintf,
	.write = debugfs_write,
	.release = debugfs_release,
};

/**
 * @brief module init function
 */
static int __init seemoo_qmi_init(void)
{
	int rc;
	
	/* create workqueue for event handling */
	work_recv_msg_count = 0;
	work_recv_msg_ind_count = 0;
	workqueue = create_singlethread_workqueue("seemoo_clnt");
	if (!workqueue)
		return -EFAULT;
	
	/* register event notifier */
	rc = qmi_svc_event_notifier_register(TEST_SERVICE_SVC_ID,
										 TEST_SERVICE_INS_ID, &seemoo_clnt_nb);
	if (rc < 0) {
		write_error("%s: notifier register failed\n", __func__);
		destroy_workqueue(workqueue);
		return rc;
	}
	
	/* create debugfs files, check for error */
	dent = debugfs_create_file("seemoo_qmi_client", 0444, 0,
							   NULL, &debug_ops);
	debugfs_create_file("seemoo_func_counter", 0444, 0,
						NULL, &debug_func_counter_ops);
	debugfs_create_file("seemoo_snprintf", 0444, 0,
						NULL, &debug_snprintf_ops);
	
	if (IS_ERR(dent)) {
		pr_err("%s: unable to create debugfs %ld\n",
			   __func__, IS_ERR(dent));
		dent = NULL;
		qmi_svc_event_notifier_unregister(TEST_SERVICE_SVC_ID,
										  TEST_SERVICE_INS_ID, &seemoo_clnt_nb);
		destroy_workqueue(workqueue);
		return -EFAULT;
	}
	
	return 0;
}

/**
 * @brief module exit function
 */
static void __exit seemoo_qmi_exit(void)
{
	qmi_svc_event_notifier_unregister(TEST_SERVICE_SVC_ID,
									  TEST_SERVICE_INS_ID, &seemoo_clnt_nb);
	destroy_workqueue(workqueue);
	debugfs_remove(dent);
}

module_init(seemoo_qmi_init);
module_exit(seemoo_qmi_exit);

MODULE_DESCRIPTION("SEEMOO QMI Client Driver");
MODULE_LICENSE("GPL v2");

