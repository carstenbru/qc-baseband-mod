/*
 UserActivityAnalyzer.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef USERACTIVITYANALYZER_H_
#define USERACTIVITYANALYZER_H_

#include "SubframeAnalyzer.h"

#define DEFAULT_INACTIVITY_TIME_MS 12000

class UserActivityAnalyzer: public SubframeAnalyzer {
public:
	UserActivityAnalyzer();
	virtual ~UserActivityAnalyzer();

	virtual bool analyze_subframe(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);

	/**
	 * sets the time (in ms) after which an RNTI is considered as inactive
	 */
	void set_inactivity_time_ms(unsigned int inactivity_time_ms) {
		this->inactivity_time_ms = inactivity_time_ms;
	}
	virtual bool set_parameter(std::string name,
			std::vector<std::string>& values);
private:
	void add_data_bits(DciResult* dci_result, unsigned int rnti);
	void define_classes(std::list<unsigned int>& classes_list,
			std::vector<std::string>& values, std::string classes_name,
			std::string unit);
	void define_classes_eq(std::list<unsigned int>& classes_list,
			std::vector<std::string>& values, std::string classes_name,
			std::string unit);
	void update_value_names(std::list<unsigned int>& classes_list,
			std::string classes_name, std::string unit);
	void classify_value_and_return(std::list<unsigned int>& classes_list, unsigned int val,
			unsigned int output_values_start);

	unsigned int inactivity_time_ms;
	bool verbose_text_output;
	unsigned int output_dcis_rnti;

	unsigned int num_active_rntis;
	uint64_t rnti_start_time[65536];
	uint64_t rnti_last_seen[65536];
	std::list<unsigned int> active_rntis;
	uint64_t rnti_bits_transmitted_dl[65536];
	uint64_t rnti_bits_transmitted_ul[65536];

	unsigned int active_time_values_start;
	unsigned int transmitted_dl_bytes_values_start;
	unsigned int transmitted_ul_bytes_values_start;
	std::list<unsigned int> classes_active_time;
	std::list<unsigned int> classes_transmitted_dl_bytes;
	std::list<unsigned int> classes_transmitted_ul_bytes;
};

#endif /* USERACTIVITYANALYZER_H_ */
