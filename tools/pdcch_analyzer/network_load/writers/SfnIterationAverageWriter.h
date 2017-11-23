/*
 SfnIterationAverageWriter.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef SFNITERATIONAVERAGEWRITER_H_
#define SFNITERATIONAVERAGEWRITER_H_

#include "ResultWriter.h"

class SfnIterationAverageWriter: public ResultWriter {
public:
	SfnIterationAverageWriter(std::string filename);
	virtual ~SfnIterationAverageWriter();
protected:
	virtual bool write_data_condition(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);
};

#endif /* SFNITERATIONAVERAGEWRITER_H_ */
