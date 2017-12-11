/*
 FrameAverageWriter.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef FRAMEAVERAGEWRITER_H_
#define FRAMEAVERAGEWRITER_H_

#include "ResultWriter.h"

class FrameAverageWriter: public ResultWriter {
public:
	FrameAverageWriter(std::string filename);
	virtual ~FrameAverageWriter();
protected:
	virtual bool write_data_condition(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);
private:
	unsigned int last_sfn;
};

#endif /* FRAMEAVERAGEWRITER_H_ */
