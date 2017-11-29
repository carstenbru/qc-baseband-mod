/*
 SubframeAnalyzer.h
 
 Abstract analyzer class. Inherit from this class to implement concrete analysis scheme.

 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef SUBFRAMEANALYZER_H_
#define SUBFRAMEANALYZER_H_

#include "records/PdcchDciRecord.h"
#include "PdcchDumpRecordReader.h"

class SubframeAnalyzer {
public:
	SubframeAnalyzer();
	virtual ~SubframeAnalyzer();

	/**
	 * analyzes a DCI (implement in subclasses!)
	 *
	 * @return true if data (results) are available, false if DCI could not be analyzed
	 */
	virtual bool analyze_subframe(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader) = 0;

	std::vector<double> get_values() {
		return values;
	}
	unsigned int get_num_samples() {
		return num_samples;
	}
	std::vector<std::string> get_value_names() {
		return value_names;
	}

	virtual bool set_parameter(std::string name, std::string value) {
		return false;
	}
protected:
	std::vector<double> values;
	unsigned int num_samples;
	std::vector<std::string> value_names;
};

#endif /* SUBFRAMEANALYZER_H_ */
