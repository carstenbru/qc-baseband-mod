/*
 PdcchMainCellInfoRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHMAINCELLINFORECORD_H_
#define PDCCHMAINCELLINFORECORD_H_

#include "PdcchDumpRecord.h"

class PdcchMainCellInfoRecord: public PdcchDumpRecord {
public:
	PdcchMainCellInfoRecord(unsigned int record_version, char* data,
			unsigned int length);
	virtual ~PdcchMainCellInfoRecord();

	virtual std::string to_string();

	uint16_t get_phy_cell_id() {
		return (*((uint32_t*) (data)) >> 20);
	}
	uint16_t get_mcc() {
		return ((*((uint32_t*) (data)) >> 10) & 0x3FF);
	}
	uint16_t get_mnc() {
		return ((*((uint32_t*) (data))) & 0x3FF);
	}
	uint16_t get_tac() {
		return ((*((uint32_t*) (data + 4))));
	}
	uint32_t get_cid() {
		return ((*((uint32_t*) (data + 8))));
	}
	/**
	 * gets the TA (timing advance) value
	 */
	uint16_t get_ta() {
		return ((*((uint32_t*) (data + 12)) >> 16));
	}
	/**
	 * gets the signal strength in ASU
	 */
	uint16_t get_asu() {
		return ((*((uint32_t*) (data + 12)) & 0x7F));
	}
	/**
	 * gets the signal strength in dBm
	 */
	uint32_t get_dBm() {
		return ((*((uint32_t*) (data + 16))));
	}

	/**
	 * returns true if two records indicate the same cell
	 */
	bool is_same_cell(PdcchMainCellInfoRecord other_cell);
};

#endif /* PDCCHMAINCELLINFORECORD_H_ */
