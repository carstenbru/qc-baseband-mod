/**
 * @file pdcch_dump.h
 * @brief PDCCH buffer dumping main header file
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
 
#ifndef __PDCCH_DUMP_H
#define __PDCCH_DUMP_H

/* service ID definitions (on top of QMI) */
#define PDCCH_DUMP_SVC_ID 0x43434864 //"CCHd" in ASCII
#define PDCCH_CELL_INFO_SVC_ID 0x43434869  //"CCHi" in ASCII

/* SEEMOO QMI service request handlers */
int pdcch_dump_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

int pdcch_cell_info_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

#endif
