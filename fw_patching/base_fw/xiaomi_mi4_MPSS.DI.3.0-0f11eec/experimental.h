 /**
 * @file experimental.h
 * @brief Definitions of existing baseband firmware functions: experimental
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __EXPERIMENTAL_H
#define __EXPERIMENTAL_H

ADDRESS(0x080BF1C0) void* pdcch_interpret(unsigned int u0, unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4, unsigned int u5); //TODO even more parameters? seems that there is a lot on the stack


ADDRESS(0x08232F60) void fw_diag_log_send_ind(unsigned int* ind_ptr, unsigned int ind_length);

ADDRESS(0x081E2830) unsigned int vpe_schdr_dl_sth(unsigned int p0, unsigned int p1);

ADDRESS(0x08019380) unsigned char* lte_LL1_get_cell_info2(unsigned int carrier_index);

ADDRESS(0x08015460) void invalidate_cache(unsigned int* address, unsigned int length);

ADDRESS(0x0959AA00) void HWIO_AllocEntry(void* pCtxt, void* pPhysEntry, void* pVirtEntry);
ADDRESS(0x09586430) void DALSYS_MemRegionAlloc(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6);

#endif 
