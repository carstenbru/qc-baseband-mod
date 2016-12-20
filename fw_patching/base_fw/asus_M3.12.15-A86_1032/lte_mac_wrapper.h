 /**
 * @file lte_mac_wrapper.h
 * @brief Definitions of existing baseband firmware LTE MAC layer functions
 * 
 * Specific to Asus Padfone Infinity 2 (A86), firmware version "M3.12.15-A86_1032"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __LTE_MAC_WRAPPER_H
#define __LTE_MAC_WRAPPER_H

ADDRESS(0x954026C) void lte_mac_dl_process_transport_block(void* transport_block);


ADDRESS(0x953FADC) void* lte_mac_dl_process_mac_pdu(void* a, void* b);

ADDRESS(0x91FD4B0) unsigned int dsm_extract_long(void* packet_ptr, unsigned int offset, void* buf, unsigned int len);
ADDRESS(0x91FD550) unsigned short dsm_extract(void* packet_ptr, unsigned short offset, void* buf, unsigned short len);

#endif
