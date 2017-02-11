/**
 * @file lte_sec.h
 * @brief LTE security/keys extracting (over QMI) patches
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __LTE_SEC_H
#define __LTE_SEC_H

/* service ID definitions (on top of QMI) */
#define LTE_SEC_SVC_ID 0x4C544573 //"LTEs" in ASCII

/* SEEMOO QMI service request handlers */
int lte_sec_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

#endif
