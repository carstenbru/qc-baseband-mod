/*
 PdcchDciRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHDCIRECORD_H_
#define PDCCHDCIRECORD_H_

#include "PdcchDataRecord.h"
#include "PdcchLlrBufferRecord.h"
#include "../pdcch_decoder/DciResult.h"

#include <list>

class PdcchDciRecord: public PdcchDataRecord {
public:
	PdcchDciRecord(unsigned int record_version, char* data, unsigned int length);
	PdcchDciRecord(PdcchLlrBufferRecord& pdcch_llr_record,
			std::list<DciResult*>* decoded_dcis);
	virtual ~PdcchDciRecord();

	virtual std::string to_string();

	int get_num_dcis() {
		return dcis_in_data_bytes ? ((*((uint8_t*) (data + 8)))) : dcis->size();
	}

	std::list<DciResult*>* get_dcis();

	virtual char* get_data();
private:
	bool dcis_in_data_bytes;
	std::list<DciResult*>* dcis;
};

#endif /* PDCCHDCIRECORD_H_ */
