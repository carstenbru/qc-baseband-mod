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
