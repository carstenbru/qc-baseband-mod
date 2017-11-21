/*
 PdcchDumpRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDumpRecord.h"

PdcchDumpRecord::PdcchDumpRecord(unsigned int record_version, char* data, unsigned int length) :
		record_version(record_version), data(data), length(length) {
}

PdcchDumpRecord::~PdcchDumpRecord() {
	delete[] data;
}

bool PdcchDumpRecord::equals(PdcchDumpRecord other) {
	if (record_version != other.record_version) {
		return false;
	}

	if (length != other.length) {
		return false;
	}

	for (unsigned int i = 0; i < length; i++) {
		if (*(data + i) != *(other.data + i)) {
			return false;
		}
	}

	return true;
}
