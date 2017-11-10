/**
 * @file seemoo_qmi_services_config.c
 * @brief configuration of service handlers for SEEMOO QMI services
 *
 * See also file seemoo_qmi_services.h
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */ 

#include "../../common/seemoo_qmi/seemoo_qmi_services.h"
#include "pdcch_dump.h"
#include "../../mem_access/src/mem_access.h"

seemoo_qmi_services_req_t seemoo_qmi_services_req[] = {
    {PDCCH_DUMP_SVC_ID, pdcch_dump_svc_req},
    {PDCCH_CELL_INFO_SVC_ID, pdcch_cell_info_svc_req},
    {MEM_ACCESS_READ_SVC_ID, mem_access_read_svc_req},
    {MEM_ACCESS_WRITE_SVC_ID, mem_access_write_svc_req},
};

int seemoo_qmi_services_req_size = sizeof(seemoo_qmi_services_req) / sizeof(seemoo_qmi_services_req_t);
