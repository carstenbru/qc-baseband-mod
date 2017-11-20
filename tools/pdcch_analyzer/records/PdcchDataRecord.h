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

	virtual std::string to_string() = 0;

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
};

#endif /* PDCCHDATARECORD_H_ */
