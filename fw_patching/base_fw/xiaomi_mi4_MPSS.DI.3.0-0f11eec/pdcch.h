/**
 * @file pdcch.h
 * @brief Definitions of existing baseband firmware functions related to PDCCH
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __PDCCH_H
#define __PDCCH_H

ADDRESS(0x08208900) void pdcch_demback_init(unsigned int subframe, unsigned int frame, unsigned int carrier_index);

ADDRESS(0x081AFB20) void config_demback_xfer_pdcch();
ADDRESS(0x081AEC10) void config_demback_xfer_pdsch();
ADDRESS(0x081AFC40) void config_demback_xfer_pbch();

ADDRESS(0x080A8000) void* pdcch_decoder_results(unsigned int frame_info);

ADDRESS(0x080C08B0) void* pdcch_demod_main(unsigned int u0, unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4, unsigned int u5);

ADDRESS(0x081B0310) void pdcch_intr();

ADDRESS(0x0822CA60) void mempool_config_ch_and_brdg(unsigned int channel, unsigned int* cfg0_cfg1_ptr);
ADDRESS(0x0822CFD0) void mempool_cpy_page(unsigned int channel, unsigned int word_offset, unsigned int* destination_address, unsigned int num_words);

#endif
