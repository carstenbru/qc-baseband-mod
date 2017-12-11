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
	/**
	 * sets the number of samples used for values (all)
	 *
	 * set to 0 to print sum instead of average
	 *
	 * @param val number of samples
	 */
	void set_num_samples(unsigned int val) {
		num_samples[0] = val;
		single_num_samples = true;
	}
	/**
	 * sets the number of samples used for values (individual for each value)
	 *
	 * set to 0 to print sum instead of average
	 *
	 * @param val_nr value for which the number of samples should be set
	 * @param val number of samples for this value
	 */
	void set_num_samples(unsigned int val_nr, unsigned int val) {
		if (val_nr > num_samples.size()) {
			num_samples.resize(val_nr + 1);
		}
		num_samples[val_nr] = val;
		single_num_samples = false;
	}
	unsigned int get_num_samples(unsigned int val_nr) {
		return (single_num_samples) ? num_samples[0] : num_samples[val_nr];
	}
	std::vector<std::string> get_value_names() {
		return value_names;
	}

	virtual bool set_parameter(std::string name, std::vector<std::string>& values) {
		return false;
	}
private:
	bool single_num_samples;
	std::vector<unsigned int> num_samples;
protected:
	std::vector<double> values;
	std::vector<std::string> value_names;
};

#endif /* SUBFRAMEANALYZER_H_ */
