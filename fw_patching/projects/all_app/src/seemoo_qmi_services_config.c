/**
 * @file seemoo_qmi_services_config.c
 * @brief configuration of service handlers for SEEMOO QMI services
 *
 * See also file seemoo_qmi_services.h
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */ 

#include "../../common/seemoo_qmi/seemoo_qmi_services.h"
#include "../../func_counter_snprintf/src/func_counter_snprintf.h"
#include "../../lte_mac/src/lte_mac.h"
#include "../../lte_sec/src/lte_sec.h"
#include "../../mem_access/src/mem_access.h"
#include "../../channel_estimation/src/channel_estimation.h"

seemoo_qmi_services_req_t seemoo_qmi_services_req[] = {
    {FUNC_COUNTER_SVC_ID, func_counter_svc_req},
    {SNPRINTF_SVC_ID, snprintf_svc_req},
    {LTE_MAC_DL_SVC_ID, lte_mac_svc_req},
    {LTE_SEC_SVC_ID, lte_sec_svc_req},
    {MEM_ACCESS_READ_SVC_ID, mem_access_read_svc_req},
    {MEM_ACCESS_WRITE_SVC_ID, mem_access_write_svc_req},
    {CHANNEL_ESTIMATION_SVC_ID, channel_estimation_svc_req}
};

int seemoo_qmi_services_req_size = sizeof(seemoo_qmi_services_req) / sizeof(seemoo_qmi_services_req_t);
