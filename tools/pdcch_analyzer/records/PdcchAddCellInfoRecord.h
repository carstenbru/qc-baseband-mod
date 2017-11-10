/*
 PdcchAddCellInfoRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHADDCELLINFORECORD_H_
#define PDCCHADDCELLINFORECORD_H_

#include "PdcchDumpRecord.h"

enum pdcch_rnti_type_t {
	C_RNTI = 0,
	SPS_C_RNTI = 1,
	P_RNTI = 2,
	RA_RNTI = 3,
	T_C_RNTI = 4,
	SI_RNTI = 5,
	TPC_PUSCH_RNTI = 6,
	TPC_PUCCH_RNTI = 7
};

class PdcchAddCellInfoRecord: public PdcchDumpRecord {
public:
	PdcchAddCellInfoRecord(unsigned int record_version, char* data,
			unsigned int length);
	virtual ~PdcchAddCellInfoRecord();

	virtual std::string to_string();

	uint16_t get_phy_cell_id() {
		return ((*((uint32_t*) (data))) & 0x1FF);
	}
	uint16_t get_bandwidth_idx() {
		return ((*((uint32_t*) (data)) >> 16));
	}
	uint16_t get_bandwidth_rb() {
		uint16_t resourceBlocks[] = { 6, 15, 25, 50, 75, 100 };
		return resourceBlocks[get_bandwidth_idx()];
	}
	float get_bandwidth_MHz() {
		float bandwidth_MHz[] = { 1.4f, 3, 5, 10, 15, 20 };
		return bandwidth_MHz[get_bandwidth_idx()];
	}
	uint16_t get_num_tx_antennas() {
		return
				(record_version > 0) ? (((*((uint32_t*) (data)) >> 11) & 0x7) + 1) : 2;
	}
	uint16_t get_num_tx_ports() {
		return get_num_tx_antennas();
	}
	uint16_t get_num_rx_antennas() {
		return (record_version > 0) ? (((*((uint32_t*) (data)) >> 9) & 0x3) + 1) : 2;
	}
	uint16_t get_num_rbs() {  //result from HW peripheral read (PDCCH_DEINT_CFG_WORD1)
		return ((*((uint32_t*) (data + 4))) & 0xFF);
	}
	/**
	 * REG (resource element group) height (number of subcarriers) in different OFDM symbols
	 *
	 * @param symb OFDM symbol (0..3)
	 */
	uint16_t get_reg_height(unsigned int symb) {
		if (symb > 3)
			return 0;
		return ((*((uint32_t*) (data + 4)) >> (8 + symb * 3)) & 0x7);
	}
	/**
	 * REGs (resource element group) per RB (resource block) in different OFDM symbols
	 *
	 * @param symb OFDM symbol (0..3)
	 */
	uint16_t get_regs_per_rb(unsigned int symb) {
		if (symb > 3)
			return 0;
		return ((*((uint32_t*) (data + 4)) >> (20 + symb * 3)) & 0x3);
	}
	/**
	 * get sizes of DCI payloads the modem tries to decode
	 *
	 * @param idx index: 0,1: common search space; 2,3: UE specific search space
	 */
	uint16_t get_configured_payload_size(unsigned int idx) {
		if (idx > 3)
			return 0;
		return ((*((uint32_t*) (data + 8)) >> (idx * 8)) & 0x7F);
	}
	/**
	 * gets a map indicating which REGs are part of PDCCH (for symbol 0)
	 * each bit stands for 1 REG location, where these locations depend on REGS_PER_RB/REG_HEIGHT
	 * if for example (REG_HEIGHT == 6), bit 20 of the map would correspond to the REG consisting of REs 121,122,124,125 (as REs 120 and 123 are excluded by the REG_HEIGHT definition)
	 * a 0 means the corresponding REG is part of PDCCH, a 1 means it is used for another channel
	 * you can for example find this back with MATLABs "ltePDCCHIndices" function (you need the correct phy CellId!) and converting the results properly
	 *
	 * @param symb OFDM symbol (0..2)
	 */
	char* get_reg_map(unsigned int symb);
	uint16_t get_num_phich_symbols() {
		return ((*((uint32_t*) (data + 216))) & 0x3);
	}
	/**
	 * gets the value of x_2[Nc+30 : Nc] of the gold-scrambling sequence (with Nc = 1600)
	 * as it only depends on the Cell_id (phy) and subframe, this can be pre-computed
	 */
	uint32_t get_pdcch_x2_scramble_seed() {
		return ((*((uint32_t*) (data + 228))));
	}
	/**
	 * gets the number of PDCCH CCEs
	 *
	 * @param cfi CFI value (1..3)
	 */
	uint32_t get_num_cce(unsigned int cfi) {
		return ((*((uint32_t*) (data + 224)) >> ((cfi - 1) * 12)) & 0x7F);
	}
	/**
	 * gets the number of PDCCH REGs modulo 9 (unusable REGs)
	 *
	 * @param cfi CFI value (1..3)
	 */
	uint32_t get_num_reg_mod_9(unsigned int cfi) {
		if (cfi == 3) {
			return ((*((uint32_t*) (data + 216)) >> 12) & 0xF);
		}
		return ((*((uint32_t*) (data + 224)) >> (8 + (cfi - 1) * 12)) & 0xF);
	}
	/**
	 * gets the total number of PDCCH REGs
	 *
	 * @param cfi CFI value (1..3)
	 */
	uint32_t get_num_regs(unsigned int cfi) {
		return get_num_cce(cfi) * 9 + get_num_reg_mod_9(cfi);
	}
	/**
	 * gets an RNTI value configured currently in the modem for decoding
	 *
	 * @param idx defines which RNTI value to return
	 */
	uint16_t get_rnti(pdcch_rnti_type_t idx) {
		return ((*((uint32_t*) (data + 232 + 4 * (idx >> 1))) >> ((idx & 1) * 16))
				& 0xFFFF);
	}
};

#endif /* PDCCHADDCELLINFORECORD_H_ */
