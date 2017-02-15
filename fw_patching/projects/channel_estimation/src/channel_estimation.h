/**
 * @file channel_estimation.h
 * @brief channel estimates extraction main source file
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
 
#ifndef __CHANNEL_ESTIMATION_H
#define __CHANNEL_ESTIMATION_H

/* service ID definitions (on top of QMI) */
#define CHANNEL_ESTIMATION_SVC_ID 0x43457374 //"CEst" in ASCII

/* SEEMOO QMI service request handlers */
int channel_estimation_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);


#endif
