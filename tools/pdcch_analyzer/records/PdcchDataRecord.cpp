/*
 PdcchDataRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDataRecord.h"
#include <iostream>
#include <math.h>
using namespace std;

PdcchDataRecord::PdcchDataRecord(unsigned int record_version, char* data,
		unsigned int length) :
		PdcchDumpRecord(record_version, data, length) {
}

PdcchDataRecord::~PdcchDataRecord() {
}

string PdcchDataRecord::to_string() {
	char buf[256];
	snprintf(buf, sizeof(buf),
			"PdcchDataRecord: phy cell ID: %d\tRBs: %d\tsfn: %04d\tsubframe: %d\tCFI: %d",
			get_phy_cell_id(), get_num_rbs(), get_sfn(), get_subframe(), get_cfi());

	return string(buf);
}

void PdcchDataRecord::get_reg_values(unsigned int reg_idx, int8_t* reg_values) {
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

void PdcchDataRecord::get_reg_bits(unsigned int reg_idx, uint8_t* reg_values) {
	for (unsigned int i = 0; i < 4; i++) {
		uint16_t val = get_buffer_word((reg_idx << 2) + 4 + i);

		*reg_values++ = (val >> 5) & 1;
		*reg_values++ = (val >> 11) & 1;
	}
}

float PdcchDataRecord::get_phich_ng() {
	unsigned int num_phich_groups = get_num_phich_groups();
	unsigned int num_rbs = get_num_rbs();
	float ng_vals[] = { 1 / 6, 1 / 2, 1, 2 };
	for (unsigned int i = 0; i < sizeof(ng_vals) / sizeof(ng_vals[0]); i++) {
		if (ceil(((float) num_rbs) / 8 * ng_vals[i]) == num_phich_groups) {
			return ng_vals[i];
		}
	}
	return 0;
}
