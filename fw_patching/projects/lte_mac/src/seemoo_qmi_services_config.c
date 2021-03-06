/**
 * @file seemoo_qmi_services_config.c
 * @brief configuration of service handlers for SEEMOO QMI services
 *
 * See also file seemoo_qmi_services.h
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */ 

#include "../../common/seemoo_qmi/seemoo_qmi_services.h"
#include "lte_mac.h"

seemoo_qmi_services_req_t seemoo_qmi_services_req[] = {
    {LTE_MAC_DL_SVC_ID, lte_mac_svc_req}
};

int seemoo_qmi_services_req_size = sizeof(seemoo_qmi_services_req) / sizeof(seemoo_qmi_services_req_t);
