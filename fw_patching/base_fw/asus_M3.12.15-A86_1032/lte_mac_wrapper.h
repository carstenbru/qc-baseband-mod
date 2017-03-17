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

ADDRESS(0x9542CD4) void lte_mac_ul_data_build_mac_pdu(void* data_struct);

ADDRESS(0x91FD4B0) unsigned int dsm_extract_long(void* packet_ptr, unsigned int offset, void* buf, unsigned int len);
ADDRESS(0x91FD550) unsigned short dsm_extract(void* packet_ptr, unsigned short offset, void* buf, unsigned short len);

ADDRESS(0x9543208) void* lte_mac_ul_data_build_msg3_ho_ul_dl_rach(void* a, void* b, void* c, void* d);

ADDRESS(0xC3B21FC) extern unsigned char lte_mac_ul_rach_struct; //actual type is some struct, not a char!


ADDRESS(0x9F15E40) void a2_ul_phy_set_technology(unsigned int technology);
ADDRESS(0x9F16B50) void a2_ul_phy_write_crc_init_task();

ADDRESS(0x9F16B44) void* a2_ul_phy_reserve_gather_task();
ADDRESS(0x9F16B38) void* a2_ul_phy_reserve_first_gather_task();
ADDRESS(0x9F16B2C) void* a2_ul_phy_reserve_first_fill_task();

ADDRESS(0x9F16A14) void a2_ul_phy_write_gather_task_to_specified_addr(void* ul_phy_task_q_addr, void* source_address, unsigned short length_in_bits);
ADDRESS(0x9F16974) void a2_ul_phy_write_first_gather_task_to_specified_addr(void* ul_phy_task_q_addr, void* source_address, void* dest_address, unsigned short dst_mem_type_p, unsigned short length_in_bits);

ADDRESS(0x9F16A98) void a2_ul_phy_write_first_fill_task_to_specified_addr(void* ul_phy_task_q_addr, void* source_address, void* dest_address, unsigned short dst_mem_type_p, unsigned short length_in_bits);

ADDRESS(0x9F16508) void a2_ul_phy_write_dsm_gather_task(void* pkt_ptr, unsigned short offset_in_bytes, unsigned short length_in_bytes);

ADDRESS(0x9F1686C) void a2_ul_phy_write_fill_task(void* source_address, unsigned short length_in_bits);

ADDRESS(0x9F164F4) void a2_ul_phy_write_padding_bits(unsigned short num_pad_bits);

ADDRESS(0x9F168CC) void a2_ul_phy_write_cipher_task(unsigned int cipher_algo, unsigned char key_index, unsigned int count_c, unsigned short key_stream_offset, unsigned char bearer_id, unsigned short cipher_length);

ADDRESS(0x9F1654C) void a2_ul_phy_write_done_task(unsigned char harq_id, unsigned short tb_size_in_bits, unsigned int interrupt_reqd);
ADDRESS(0x9F165CC) void a2_ul_phy_commit_write_ptr(unsigned char harq_id);

ADDRESS(0x9545AF0) void lte_mac_rach_send_preamble();
ADDRESS(0x9545C38) void lte_mac_rach_retry_preamble();

ADDRESS(0x9F16794) void a2_ul_phy_harq_init(unsigned char harq_id, unsigned int univ_stmr_val);
ADDRESS(0x9F16CC8) void a2_ul_phy_process_harq_retx(unsigned char harq_id, unsigned int univ_stmr_val, unsigned int enc_addr);
ADDRESS(0x9F16B60) void a2_ul_phy_process_harq_dtx(unsigned char harq_id);

#endif
