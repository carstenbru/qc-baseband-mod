/**
 * @file lte_mac.h
 * @brief LTE MAC Layer messages forwarding (over QMI) patches
 *
 * This projects also includes the features of the func_counter_snprintf project.
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __LTE_MAC_H
#define __LTE_MAC_H

/* service ID definitions (on top of QMI) */
#define LTE_MAC_DL_SVC_ID 0x4D414364 //"MACd" in ASCII
#define LTE_MAC_UL_SVC_ID 0x4D414375 //"MACu" in ASCII
#define LTE_MAC_UL_RACH_SVC_ID 0x4D414372 //"MACr" in ASCII

/* SEEMOO QMI service request handlers */
int lte_mac_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data);

#endif
