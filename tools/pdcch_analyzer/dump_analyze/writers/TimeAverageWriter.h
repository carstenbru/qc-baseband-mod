/*
 TimeAverageWriter.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef TIMEAVERAGEWRITER_H_
#define TIMEAVERAGEWRITER_H_

#include "ResultWriter.h"

class TimeAverageWriter: public ResultWriter {
public:
	TimeAverageWriter(std::string filename, unsigned long time_interval);
	virtual ~TimeAverageWriter();
protected:
	virtual bool write_data_condition(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);
private:
	unsigned long time_interval;
	unsigned long written_last_time;
};

#endif /* TIMEAVERAGEWRITER_H_ */
