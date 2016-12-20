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

//TODO docs

#include <stdarg.h>

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/qmi_encdec.h>

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

#include <asm/uaccess.h>

#include <mach/msm_qmi_interface.h>

#include "kernel_test_service_v01.h"

#include "seemoo_qmi_client_int.h"

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

/* write buffer for status device, will be returned on next read */
unsigned char status_buf[WRITE_BUF_SIZE];
int status_pos;

/* read buffer for packets on data device */
unsigned char read_buf[WRITE_BUF_SIZE];

/* packet buffer for data device, one packet will be returned on next read */
seemoo_qmi_packet_t* data_list;
seemoo_qmi_packet_t** data_list_last_item_pointer;

/* ############################### */

/**
 * @brief writes a text into the write buffer, call like printf
 */
static void write_text(char* format, ...) {
	va_list args;
	va_start(args, format);
	
	status_pos += vsnprintf(status_buf+status_pos, sizeof(status_buf)-status_pos, format, args);
	
	va_end(args);
}

/**
 * @brief writes a text into the error buffer, call like printf
 */
static void write_error(char* format, ...) {
	va_list args;
	va_start(args, format);
	
	status_pos += vsnprintf(status_buf+status_pos, sizeof(status_buf)-status_pos, format, args);
	
	va_end(args);
}

static void received_message(unsigned char* data, unsigned int data_len, unsigned int indication) {
    seemoo_qmi_packet_t* qmi_packet;
    if (indication) {
        *(data + 3) |= 0x80; //set first bit of SVC ID to 1 to indicate an indication
    }
    
    qmi_packet = kzalloc(sizeof(seemoo_qmi_packet_t), GFP_KERNEL);
    qmi_packet->next = NULL;
    qmi_packet->length = data_len;
    memcpy(qmi_packet->data, data, data_len);
    
    *data_list_last_item_pointer = qmi_packet;
    data_list_last_item_pointer = &qmi_packet->next;
}

/**
 * @brief sends a synchronous (i.e. waits for the result) message to a service
 *
 * @param handle client handle to use
 * @param data_len payload data length
 * @param data payload data as array of bytes
 */
static int services_send_sync_msg(struct qmi_handle* handle, unsigned int data_len, unsigned char* data) {
	struct test_data_req_msg_v01* req;
	struct test_data_resp_msg_v01* resp;
	struct msg_desc req_desc, resp_desc;
	int rc;
	
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
	
	req->data_len = data_len;
	if (data_len) { //include payload data (if passed)
		memcpy(req->data, data, data_len);
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
	
	received_message(resp->data, resp->data_len, 0);
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
		if (rc < 0) {
			write_error("%s: Error receiving message\n", __func__);
        }
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
						enum qmi_event_type event, void *notify_priv) {
	int handle_id = (int)notify_priv;
	switch (event) {
		case QMI_RECV_MSG:
			// check for which handler this notification is
			if (handle_id == HANDLE_ID_NORM) {
				work_recv_msg_count++;
				queue_delayed_work(workqueue, &work_recv_msg, 0);
			} else if (handle_id == HANDLE_ID_IND) {
				work_recv_msg_ind_count++;
				queue_delayed_work(workqueue, &work_recv_msg_ind, 0);
			}
			break;
		default:
			break;
	}
}

/**
 * @brief indication receive callback function
 * 
 * @param handle origin handle
 * @param msg_id message service ID
 * @param msg pointer to undecoded (!) message
 * @param msg_len length of the message in bytes
 */
static void indication_cb(struct qmi_handle* handle, unsigned int msg_id, void* msg, unsigned int msg_len, void* ind_cb_priv) {
	struct msg_desc ind_desc;
	test_data_ind_msg_v01* ind;
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
		
		received_message(ind->data, ind->data_len, 1);
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
	rc = qmi_connect_to_service(seemoo_clnt, TEST_SERVICE_SVC_ID, TEST_SERVICE_INS_ID);
	qmi_connect_to_service(seemoo_clnt_ind, TEST_SERVICE_SVC_ID, TEST_SERVICE_INS_ID);
	if (rc < 0) {
		write_error("%s: Server not found\n", __func__);
		qmi_handle_destroy(seemoo_clnt);
		seemoo_clnt = NULL;
		return;
	}
	
	/* register callback function for indications */
	rc = qmi_register_ind_cb(seemoo_clnt_ind, indication_cb, NULL);
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
static void svc_exit(struct work_struct *work) {
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
static int svc_event_notify(struct notifier_block *this, unsigned long code, void *_cmd) {
	switch (code) {
		case QMI_SERVER_ARRIVE:
			queue_delayed_work(workqueue, &work_svc_arrive, 0);
			break;
		case QMI_SERVER_EXIT:
			queue_delayed_work(workqueue, &work_svc_exit, 0);
			break;
		default:
			break;
	}
	return 0;
}

static int dev_open(struct inode *ip, struct file *fp) {
	if (!seemoo_clnt) {
		write_error("%s SEEMOO client is not initialized\n", __func__);
		return -ENODEV;
	}
	return 0;
}

static ssize_t status_read(struct file *fp, char __user *buf, size_t count, loff_t *pos) {
	ssize_t s = simple_read_from_buffer(buf, count, pos, status_buf, strnlen(status_buf, WRITE_BUF_SIZE));
	status_buf[0] = 0;
	status_pos = 0;
	return s;
}

static ssize_t data_read(struct file *fp, char __user *buf, size_t count, loff_t *pos) {
    unsigned int len;
    
    if (data_list != NULL) {
        seemoo_qmi_packet_t* packet = data_list;
        data_list = data_list->next;
        if (data_list == NULL) {
            data_list_last_item_pointer = &data_list;
        }
        
        len = packet->length;
        if (copy_to_user(buf, packet->data, len)) {
            write_error("error copying packet to user\n");
        }
        
        kfree(packet);
        
        return len;
    }
    return 0;
}

static ssize_t data_write(struct file *fp, const char __user *buf, size_t count, loff_t *pos) {
	int len;
	
	if (count < 1)
		return 0;
	
	len = min(count, (sizeof(read_buf) - 1));
	
	if (copy_from_user(read_buf, buf, len))
		return -EFAULT;
    	
    if ((read_buf[3] & 0x80) == 0) {
        services_send_sync_msg(seemoo_clnt, len, read_buf);
    } else {
        read_buf[3] &= 0x7F;
        services_send_sync_msg(seemoo_clnt_ind, len, read_buf);
    }
	
	return count;
}

static struct notifier_block seemoo_clnt_nb = {
	.notifier_call = svc_event_notify,
};

static const struct file_operations status_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.read = status_read,
};

static struct miscdevice status_dev = {
	MISC_DYNAMIC_MINOR,
	"seemoo_qmi_status",
	&status_fops
};

static const struct file_operations data_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.read = data_read,
	.write = data_write,
};

static struct miscdevice data_dev = {
	MISC_DYNAMIC_MINOR,
	"seemoo_qmi_data",
	&data_fops
};

/**
 * @brief module init function
 */
static int __init seemoo_qmi_init(void)
{
	int rc, ret_status, ret_data;
	
    data_list = NULL;
    data_list_last_item_pointer = &data_list;
    
	/* create workqueue for event handling */
	work_recv_msg_count = 0;
	work_recv_msg_ind_count = 0;
	workqueue = create_singlethread_workqueue("seemoo_clnt");
	if (!workqueue)
		return -EFAULT;
	
	/* register event notifier */
	rc = qmi_svc_event_notifier_register(TEST_SERVICE_SVC_ID, TEST_SERVICE_INS_ID, &seemoo_clnt_nb);
	if (rc < 0) {
		write_error("%s: notifier register failed\n", __func__);
		destroy_workqueue(workqueue);
		return rc;
	}
	
	/* create device files, check for error */
	ret_status = misc_register(&status_dev);
    ret_data = misc_register(&data_dev);
	
	if (ret_status | ret_data) {
		pr_err("%s: unable to create device files (%d, %d)\n", __func__, ret_status, ret_data);
		qmi_svc_event_notifier_unregister(TEST_SERVICE_SVC_ID, TEST_SERVICE_INS_ID, &seemoo_clnt_nb);
		destroy_workqueue(workqueue);
		return -EFAULT;
	}
	
	return 0;
}

/**
 * @brief module exit function
 */
static void __exit seemoo_qmi_exit(void) {
	qmi_svc_event_notifier_unregister(TEST_SERVICE_SVC_ID, TEST_SERVICE_INS_ID, &seemoo_clnt_nb);
	destroy_workqueue(workqueue);
	misc_deregister(&status_dev);
    misc_deregister(&data_dev);
}

module_init(seemoo_qmi_init);
module_exit(seemoo_qmi_exit);

MODULE_DESCRIPTION("SEEMOO QMI Client Driver");
MODULE_LICENSE("GPL v2");

