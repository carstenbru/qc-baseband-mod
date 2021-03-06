/*
 PdcchLlrBufferRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchLlrBufferRecord.h"

using namespace std;

PdcchLlrBufferRecord::PdcchLlrBufferRecord(unsigned int record_version, char* data,
		unsigned int length) :
		PdcchDataRecord(record_version, data, length) {
}

PdcchLlrBufferRecord::~PdcchLlrBufferRecord() {
}

string PdcchLlrBufferRecord::to_string() {
	char buf[256];
	snprintf(buf, sizeof(buf),
			"PdcchLlrBufferRecord: phy cell ID: %d\tRBs: %d\tsfn: %04d\tsubframe: %d\tCFI: %d",
			get_phy_cell_id(), get_num_rbs(), get_sfn(), get_subframe(), get_cfi());

	return string(buf);
}

void PdcchLlrBufferRecord::get_reg_values(unsigned int reg_idx, int8_t* reg_values) {
	for (unsigned int i = 0; i < 4; i++) {
		uint16_t val = get_buffer_word((reg_idx << 2) + 4 + i);

		*reg_values = (val & 0x3F);
		if (*reg_values & 0x20) {
			*reg_values |= ~0x3F;
		}
		reg_values++;
		*reg_values = ((val >> 6) & 0x3F);
		if (*reg_values & 0x20) {
			*reg_values |= ~0x3F;
		}
		reg_values++;
	}
}

void PdcchLlrBufferRecord::get_reg_bits(unsigned int reg_idx, uint8_t* reg_values) {
	for (unsigned int i = 0; i < 4; i++) {
		uint16_t val = get_buffer_word((reg_idx << 2) + 4 + i);

		*reg_values++ = (val >> 5) & 1;
		*reg_values++ = (val >> 11) & 1;
	}
}
