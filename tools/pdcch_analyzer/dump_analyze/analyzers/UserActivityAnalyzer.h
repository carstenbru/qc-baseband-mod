/*
 UserActivityAnalyzer.h
 
 Analyzer which calculates statistics about user behavior.

 In particular, it observes when RNTIs get active and inactive and counts the number of active RNTIs
 in the cell at each subframe. As inactivity can only be detected after an inactivity timer expired, these
 three output values ("active RNTIs", "RNTIs got active", "RNTIs got inactive") are delayed by some milliseconds,
 defined by the parameter "inactivity_time_ms".

 In addition, the analyzer can output the activity time of RNTIs and the consumed data (downlink & uplink) during
 this period. Keep in mind that a base station will disconnect an UE when it is inacitve for some time (~12s) and
 for its next activity a new RNTI will be assigned.
 Thus, the same user/UE can be active with different RNTI values. The ouput values only represent the activity time
 and data consumption in a SINGLE "BURST" of communication without breaks.
 Re-transmissions are excluded from the results, so only initial grants are considered.

 The first few output values should be dropped, as they might be incorrect, e.g. the activity time might be
 too short as the UE might have been active before the dump started.

 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef USERACTIVITYANALYZER_H_
#define USERACTIVITYANALYZER_H_

#include "SubframeAnalyzer.h"

#define DEFAULT_INACTIVITY_TIME_MS 20000
#define DEFAULT_BURST_BREAK_INACTIVITY_MS 640

typedef struct {
	unsigned int rnti;
	uint64_t timestamp;
} rnti_got_active_event_t;

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
	bool add_data_bits(DciResult* dci_result, unsigned int rnti);
	void define_classes(std::list<unsigned int>& classes_list,
			std::vector<std::string>& values, std::string classes_name, float divide,
			std::string unit);
	void define_classes_eq(std::list<unsigned int>& classes_list,
			std::vector<std::string>& values, std::string classes_name, float divide,
			std::string unit);
	void update_value_names(std::list<unsigned int>& classes_list,
			std::string classes_name, float divide, std::string unit);
	void update_rnti_value_names();
	void classify_value_and_return(std::list<unsigned int>& classes_list,
			unsigned int val, unsigned int output_values_start);

	void clear_values();
	/**
	 * checks all received DCIs for new RNTIs and updates data consumption counters for known ones
	 */
	void evaluate_dcis(uint64_t current_time, PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);
	/**
	 * processes the queue of events of RNTIs which got active. This implements the delay for the active detection, to synchronize acitve and inactive detection
	 * and thus to enable counting the number of active RNTIs at each timestep
	 */
	unsigned int process_rnti_active_queue(uint64_t current_time);
	/**
	 * checks if RNTIs got inactive, updates output values accordingly and returns the number of RNTIs which got inactive
	 */
	unsigned int check_rntis_inactive_set_values(uint64_t current_time);

	unsigned int inactivity_time_ms;
	bool verbose_text_output;
	unsigned int output_dcis_rnti;
	bool output_headers_kb;
	bool output_headers_s;

	std::list<rnti_got_active_event_t> rnti_got_active_events;
	unsigned int num_active_rntis;
	uint64_t rnti_start_time[65536];
	uint64_t rnti_last_seen[65536];
	std::list<unsigned int> active_rntis;
	uint64_t rnti_bits_transmitted_dl[65536];
	uint64_t rnti_bits_transmitted_ul[65536];

	unsigned int burst_break_inactivity_ms;
	uint64_t rnti_last_burst_end[65536];
	uint64_t rnti_burst_start[65536];
	uint64_t rnti_burst_end[65536];

	unsigned int network_disconnect_max_burst_size;
	unsigned int active_time_values_start;
	unsigned int transmitted_dl_bytes_values_start;
	unsigned int transmitted_ul_bytes_values_start;
	std::list<unsigned int> classes_active_time;
	std::list<unsigned int> classes_transmitted_dl_bytes;
	std::list<unsigned int> classes_transmitted_ul_bytes;
};

#endif /* USERACTIVITYANALYZER_H_ */
