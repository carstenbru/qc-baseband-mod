/**
 * @file func_counter_snprintf.h
 * @brief memory access (read/write) example main header file
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
 
#ifndef __MEM_ACCESS_H
#define __MEM_ACCESS_H

/* service ID definitions (on top of QMI) */
#define MEM_ACCESS_READ_SVC_ID 0x4D454D66 //"MEMr" in ASCII
#define MEM_ACCESS_WRITE_SVC_ID 0x4D454D77 //"MEMw" in ASCII

/* SEEMOO QMI service request handlers */
int mem_access_read_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

int mem_access_write_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

#endif
