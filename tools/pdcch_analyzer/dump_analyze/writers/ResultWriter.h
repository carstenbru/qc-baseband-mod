/*
 ResultWriter.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef RESULTWRITER_H_
#define RESULTWRITER_H_

#include "../analyzers/SubframeAnalyzer.h"

class ResultWriter {
public:
	ResultWriter(std::string filename);
	virtual ~ResultWriter();

	void add_analyzer(SubframeAnalyzer* subframe_analyzer);
	void new_results(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);
	void clear_samples();
	unsigned int get_num_subframes() {
		return num_subframes;
	}
	void set_write_timestamps(bool write_timestamps) {
		this->write_timestamps = write_timestamps;
	}
	void set_write_sfn_iteration(bool write_sfn_iteration) {
		this->write_sfn_iteration = write_sfn_iteration;
	}
	void set_write_sfn(bool write_sfn) {
		this->write_sfn = write_sfn;
	}
protected:
	virtual bool write_data_condition(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader) = 0;
private:
	void write_file_header(PdcchDumpRecordReader* pdcch_dump_record_reader);

	std::ofstream file_stream;

	std::list<SubframeAnalyzer*> analyzers;
	std::vector<unsigned int> num_samples;

	bool first_write;
	std::vector<double> values;

	bool write_timestamps;
	bool write_sfn_iteration;
	bool write_sfn;

	unsigned int num_subframes;
};

#endif /* RESULTWRITER_H_ */
