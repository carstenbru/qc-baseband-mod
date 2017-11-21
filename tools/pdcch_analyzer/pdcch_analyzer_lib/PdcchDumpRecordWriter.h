/*
 PdcchDumpRecordWriter.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHDUMPRECORDWRITER_H_
#define PDCCHDUMPRECORDWRITER_H_

#include "records/PdcchDumpRecord.h"

#include <string>
#include <fstream>

class PdcchDumpRecordWriter {
public:
	PdcchDumpRecordWriter(std::string filename);
	virtual ~PdcchDumpRecordWriter();

	void set_split_size(unsigned int split_size) {
		this->split_size = split_size;
	}
	void write_record(PdcchDumpRecord* record);
private:
	unsigned int split_size;
	unsigned int bytes_written;
	unsigned int cur_split_file;

	std::string base_filename;
	std::ofstream file_stream;
};

#endif /* PDCCHDUMPRECORDWRITER_H_ */
