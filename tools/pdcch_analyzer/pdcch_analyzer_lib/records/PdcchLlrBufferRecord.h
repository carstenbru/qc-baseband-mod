/*
 PdcchLlrBufferRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHLLRBUFFERRECORD_H_
#define PDCCHLLRBUFFERRECORD_H_

#include "PdcchDataRecord.h"

class PdcchLlrBufferRecord: public PdcchDataRecord {
public:
	PdcchLlrBufferRecord(unsigned int record_version, char* data,
			unsigned int length);
	virtual ~PdcchLlrBufferRecord();

	virtual std::string to_string();

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

#endif /* PDCCHLLRBUFFERRECORD_H_ */
