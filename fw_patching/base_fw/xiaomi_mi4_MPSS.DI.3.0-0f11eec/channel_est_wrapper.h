 /**
 * @file channel_est_wrapper.h
 * @brief Definitions of existing baseband firmware channel estimation functions
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __CHANNEL_EST_WRAPPER_H
#define __CHANNEL_EST_WRAPPER_H

ADDRESS(0x08222760) void lte_LL1_log_csf_whitened_matrices(unsigned int start_system_sub_frame_number_sfn, unsigned int start_system_frame_number, unsigned int num_whiten_matrices_for_csf, unsigned int num_txant, unsigned int num_rxant, unsigned char* matrices_address);

ADDRESS(0x081F4380) void lte_LL1_csf_callback(unsigned int carrier_index, unsigned int system_frame_number, unsigned int sub_frame_number);

ADDRESS(0x081E5EC0) void* lte_LL1_get_cell_info(unsigned char carrier_index);

ADDRESS(0x0C483468) extern unsigned char* lte_LL1_csf_config_pointer; //actual pointer target type is some struct, not a char!

#endif
 
