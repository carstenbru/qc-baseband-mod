/*
 PdcchDataRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHDATARECORD_H_
#define PDCCHDATARECORD_H_

#include "PdcchDumpRecord.h"

class PdcchDataRecord: public PdcchDumpRecord {
public:
	PdcchDataRecord(unsigned int record_version, char* data, unsigned int length);
	virtual ~PdcchDataRecord();

	virtual std::string to_string();

	/**
	 * returns the current CFI value (1..3)
	 */
	uint16_t get_cfi() {
		return ((*((uint32_t*) (data)) >> 30) & 0x3) + 1;
	}
	uint16_t get_phy_cell_id() {
		return ((*((uint32_t*) (data)) >> 16) & 0x1FF);
	}
	/**
	 * gets the system frame number (SFN) at which the data was obtained
	 */
	uint16_t get_sfn() {
		return ((*((uint32_t*) (data)) >> 4) & 0x3FF);
	}
	/**
	 * gets the subframe number at which the data was obtained
	 */
	uint16_t get_subframe() {
		return ((*((uint32_t*) (data))) & 0xF);
	}
	/**
	 * gets the current C-RNTI of the modem
	 */
	uint16_t get_ue_crnti() {
		return ((*((uint32_t*) (data + 4)) >> 16));
	}
	uint16_t get_phich_duration() {
		return ((*((uint32_t*) (data + 4)) >> 8) & 0x3);
	}
	uint16_t get_num_phich_groups() {
		return ((*((uint32_t*) (data + 4)) >> 10) & 0x3F);
	}
	float get_phich_ng();

	uint16_t get_num_rbs() {
		return ((*((uint32_t*) (data + 4))) & 0xFF);
	}

	/**
	 * gets a word (12bit) from the buffer
	 *
	 * @param offset word offset in buffer
	 */
	inline uint16_t get_buffer_word(unsigned int offset) {
		unsigned int byte_offset = offset + offset / 2;
		uint8_t b0 = *((uint8_t*) data + 8 + byte_offset);
		uint8_t b1 = *((uint8_t*) data + 9 + byte_offset);
		return (offset & 1) ? (b1 << 4) | (b0 >> 4) : (((b1 & 0xF) << 8) | b0);
	}

	/**
	 * gets the LLR (6-bit signed) values of all bits in the requested REG (resource element group)
	 *
	 * In these values, -31 means a 1 was received for sure, 31 means a 0 was detected
	 *
	 * @param reg_idx REG position
	 * @param reg_values result LLR values for the 8 bits of the REG
	 */
	void get_reg_values(unsigned int reg_idx, int8_t* reg_values);
	/**
	 * gets the (hard-decision) values of all bits in the requested REG (resource element group)
	 *
	 * Usually, you should use get_reg_values instead of this function!
	 *
	 * @param reg_idx REG position
	 * @param reg_values result LLR values for the 8 bits of the REG
	 */
	void get_reg_bits(unsigned int reg_idx, uint8_t* reg_values);
};

#endif /* PDCCHDATARECORD_H_ */
