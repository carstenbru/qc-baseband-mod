/**
 * @file func_counter_snprintf.h
 * @brief function counters and snprintf example main header file
 *
 * framework capabilities demonstration example, shows:
 * -function overwriting
 * -placement in pointer tables
 * -calling of firmware function
 * -calling of overwritten (original) firmware function
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
 
#ifndef __FUNC_COUNTER_SNPRINTF_H
#define __FUNC_COUNTER_SNPRINTF_H

/* service ID definitions (on top of QMI) */
#define FUNC_COUNTER_SVC_ID 0x4675436F //"FuCo" in ASCII
#define SNPRINTF_SVC_ID 0x736E7066 //"snpf" in ASCII

/* SEEMOO QMI service request handlers */
int func_counter_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

int snprintf_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

#endif
