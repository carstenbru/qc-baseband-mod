/*
 UserActivityAnalyzer.cpp
 
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
#include "UserActivityAnalyzer.h"

#include <iostream>

using namespace std;

UserActivityAnalyzer::UserActivityAnalyzer() :
		inactivity_time_ms(DEFAULT_INACTIVITY_TIME_MS), verbose_text_output(false), output_dcis_rnti(
				0), output_headers_kb(false), output_headers_s(false), num_active_rntis(
				0), burst_break_inactivity_ms(DEFAULT_BURST_BREAK_INACTIVITY_MS), active_time_values_start(
				0), network_disconnect_max_burst_size(0), transmitted_dl_bytes_values_start(
				0), transmitted_ul_bytes_values_start(0) {
	for (unsigned int i = 0; i < 65536; i++) {
		rnti_start_time[i] = 0;
		rnti_last_seen[i] = 0;
		rnti_bits_transmitted_dl[i] = 0;
		rnti_bits_transmitted_ul[i] = 0;
	}

	set_num_samples(0, 1);
	set_num_samples(1, 0);
	set_num_samples(2, 0);
	value_names.resize(3);
	update_rnti_value_names();

	values.resize(3);
}

UserActivityAnalyzer::~UserActivityAnalyzer() {
}

void UserActivityAnalyzer::update_rnti_value_names() {
	char name_buf[512];
	snprintf(name_buf, 512, "active RNTIs (delayed: %dms)", inactivity_time_ms);
	value_names[0] = name_buf;
	snprintf(name_buf, 512, "RNTIs got active (delayed: %dms)",
			inactivity_time_ms);
	value_names[1] = name_buf;
	snprintf(name_buf, 512, "RNTIs got inactive (delayed: %dms)",
			inactivity_time_ms);
	value_names[2] = name_buf;
}

void UserActivityAnalyzer::update_value_names(list<unsigned int>& classes_list,
		string classes_name, float divide, string unit) {
	unsigned int output_values_start = value_names.size();

	unsigned int lower = classes_list.front();
	char name_buf[512];
	snprintf(name_buf, 512, "%s [0, %.6g%s]", classes_name.c_str(),
			lower / divide, unit.c_str());
	value_names.push_back(name_buf);
	for (list<unsigned int>::iterator it = (++classes_list.begin());
			it != classes_list.end(); it++) {
		unsigned int upper = *it;
		snprintf(name_buf, 512, "%s (%.6g%s, %.6g%s]", classes_name.c_str(),
				lower / divide, unit.c_str(), upper / divide, unit.c_str());
		value_names.push_back(name_buf);
		lower = upper;
	}
	snprintf(name_buf, 512, "%s (%.6g%s, inf]", classes_name.c_str(),
			lower / divide, unit.c_str());
	value_names.push_back(name_buf);

	for (unsigned int i = output_values_start + classes_list.size();
			i >= output_values_start; i--) {
		set_num_samples(i, 0);  //TODO also support normalized output? maybe normalized to class width, to easily plot real histograms
	}
	values.resize(value_names.size());
}

void UserActivityAnalyzer::define_classes(list<unsigned int>& classes_list,
		vector<string>& values, string classes_name, float divide, string unit) {
	for (unsigned int i = 1; i < values.size(); i++) {
		int int_val = atoi(values[i].c_str());
		classes_list.push_back(int_val);
	}
	update_value_names(classes_list, classes_name, divide, unit);
}

void UserActivityAnalyzer::define_classes_eq(list<unsigned int>& classes_list,
		vector<string>& values, string classes_name, float divide, string unit) {
	unsigned int intervals = atoi(values[2].c_str());
	float interval_size = atoi(values[1].c_str()) / (float) intervals;
	float int_border = 0;
	while (intervals--) {
		int_border += interval_size;
		classes_list.push_back((unsigned int) int_border);
	}
	update_value_names(classes_list, classes_name, divide, unit);
}

bool UserActivityAnalyzer::set_parameter(string name, vector<string>& values) {
	if (SubframeAnalyzer::set_parameter(name, values)) {
		return true;
	}
	if (name.compare("inactivity_time_ms") == 0) {
		int int_val = atoi(values[1].c_str());
		set_inactivity_time_ms(int_val);
		update_rnti_value_names();
	} else if (name.compare("burst_break_inactivity_ms") == 0) {
		int int_val = atoi(values[1].c_str());
		burst_break_inactivity_ms = int_val;
	} else if (name.compare("network_disconnect_max_burst_size") == 0) {
		int int_val = atoi(values[1].c_str());
		network_disconnect_max_burst_size = int_val;
	} else if (name.compare("verbose_text_output") == 0) {
		int int_val = atoi(values[1].c_str());
		verbose_text_output = int_val;
	} else if (name.compare("output_dcis_rnti") == 0) {
		int int_val = atoi(values[1].c_str());
		output_dcis_rnti = int_val;
	} else if (name.compare("output_headers_kb") == 0) {
		int int_val = atoi(values[1].c_str());
		output_headers_kb = int_val;
	} else if (name.compare("output_headers_s") == 0) {
		int int_val = atoi(values[1].c_str());
		output_headers_s = int_val;
	} else if (name.compare("classes_active_time") == 0) {
		active_time_values_start = value_names.size();
		if (output_headers_s) {
			define_classes(classes_active_time, values, "active time", 1000, "s");
		} else {
			define_classes(classes_active_time, values, "active time", 1, "ms");
		}
	} else if (name.compare("classes_transmitted_dl_bytes") == 0) {
		transmitted_dl_bytes_values_start = value_names.size();
		if (output_headers_kb) {
			define_classes(classes_transmitted_dl_bytes, values, "DL transmitted",
					1000, "kB");
		} else {
			define_classes(classes_transmitted_dl_bytes, values, "DL transmitted", 1,
					"bytes");
		}
	} else if (name.compare("classes_transmitted_ul_bytes") == 0) {
		transmitted_ul_bytes_values_start = value_names.size();
		if (output_headers_kb) {
			define_classes(classes_transmitted_ul_bytes, values, "UL transmitted",
					1000, "kB");
		} else {
			define_classes(classes_transmitted_ul_bytes, values, "UL transmitted", 1,
					"bytes");
		}
	} else if (name.compare("classes_active_time_eq") == 0) {
		active_time_values_start = value_names.size();
		if (output_headers_s) {
			define_classes_eq(classes_active_time, values, "active time", 1000, "s");
		} else {
			define_classes_eq(classes_active_time, values, "active time", 1, "ms");
		}
	} else if (name.compare("classes_transmitted_dl_bytes_eq") == 0) {
		transmitted_dl_bytes_values_start = value_names.size();
		if (output_headers_kb) {
			define_classes_eq(classes_transmitted_dl_bytes, values, "DL transmitted",
					1000, "kB");
		} else {
			define_classes_eq(classes_transmitted_dl_bytes, values, "DL transmitted",
					1, "bytes");
		}
	} else if (name.compare("classes_transmitted_ul_bytes_eq") == 0) {
		transmitted_ul_bytes_values_start = value_names.size();
		if (output_headers_kb) {
			define_classes_eq(classes_transmitted_ul_bytes, values, "UL transmitted",
					1000, "kB");
		} else {
			define_classes_eq(classes_transmitted_ul_bytes, values, "UL transmitted",
					1, "bytes");
		}
	}
	return false;
}

bool is_c_rnti(unsigned int rnti) {
	return ((rnti >= 10) && (rnti <= 0xFFF3));
}

bool UserActivityAnalyzer::add_data_bits(DciResult* dci_result,
		unsigned int rnti) {
	bool contains_data_grant = false;
	/* get (downlink) grant encoded in DCI and add to counters */
	srslte_ra_dl_grant_t* dl_grant = dci_result->get_dl_grant();
	if (dl_grant != 0) {
		unsigned int d_bits = 0;
		for (int i = 0; i < SRSLTE_MAX_CODEWORDS; i++) {
			if (dl_grant->tb_en[i]) {
				if ((dl_grant->mcs[i].tbs != -1) && (dl_grant->mcs[i].idx < 29)) {
					d_bits += dl_grant->mcs[i].tbs;
				}
			}
		}
		rnti_bits_transmitted_dl[rnti] += d_bits;
		contains_data_grant = (d_bits != 0);
		if (output_dcis_rnti != 0) {
			if (rnti == output_dcis_rnti) {
				cout << " DL: " << d_bits << endl;
			}
		}
	} else {
		/* get (uplink) grant encoded in DCI and get MCS */
		srslte_ra_ul_grant_t* ul_grant = dci_result->get_ul_grant();
		if (ul_grant != 0) {
			if (ul_grant->mcs.idx < 29) {  //remove re-transmissions and only-CQI requests
				rnti_bits_transmitted_ul[rnti] += ul_grant->mcs.tbs;
				contains_data_grant = true;
			}
			if (output_dcis_rnti != 0) {
				if (rnti == output_dcis_rnti) {
					if (ul_grant->mcs.idx < 29) {
						cout << " UL: " << ul_grant->mcs.tbs << endl;
					} else {
						cout << " no (new) data" << endl;
					}
				}
			}
		}
	}
	return contains_data_grant;
}

void UserActivityAnalyzer::classify_value_and_return(
		list<unsigned int>& classes_list, unsigned int val,
		unsigned int output_values_start) {
	unsigned int val_pos = output_values_start;
	if (output_values_start) {
		for (list<unsigned int>::iterator it = classes_list.begin();
				it != classes_list.end(); it++) {
			if (val <= *it) {  //find first class with upper border smaller than our value
				break;
			}
			val_pos++;
		}
		values[val_pos]++;
	}
}

void UserActivityAnalyzer::clear_values() {
	for (unsigned int val_pos = 3; val_pos < value_names.size(); val_pos++) {  //we can skip the first 3 values
		values[val_pos] = 0;
	}
}

void UserActivityAnalyzer::evaluate_dcis(uint64_t current_time,
		PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
			it != dci_record->get_dcis()->end(); it++) {
		DciResult* dci_result = *it;

		unsigned int rnti = dci_result->get_rnti();

		if (rnti_start_time[rnti] == 0) {  //RNTI got active
			rnti_start_time[rnti] = current_time;  //store start time of activity
			active_rntis.push_back(rnti);  //add to list of active RNTIs
			if (is_c_rnti(rnti)) {  //only include c_rnti values in output data
				rnti_got_active_event_t event = { rnti, current_time };
				rnti_got_active_events.push_back(event);
				if (verbose_text_output) {
					if (rnti_last_seen[rnti] == 0) {
						cout << "+++ RNTI got active (first-time): " << rnti << endl;
					} else {
						cout << "### RNTI got active (re-used): " << rnti << endl;
					}
				}
			}
			rnti_bits_transmitted_dl[rnti] = 0;  //reset data counters
			rnti_bits_transmitted_ul[rnti] = 0;
			rnti_burst_start[rnti] = 0;  // reset data burst detection variables
			rnti_burst_end[rnti] = 0;
			rnti_last_burst_end[rnti] = 0;
		}
		if (output_dcis_rnti != 0) {
			if (rnti == output_dcis_rnti) {
				cout << "!!! DCI for " << rnti << ", it: "
						<< pdcch_dump_record_reader->get_sfn_iteration() << " SFN: "
						<< dci_record->get_sfn() << " subframe: "
						<< dci_record->get_subframe() << " "
						<< dci_result->get_format_as_string();
			}
		}
		bool contains_data_grant = add_data_bits(dci_result, rnti);

		if (contains_data_grant) {  //data burst detection
			//TODO at end: evaluate bursts data and report correct time
			if ((rnti_burst_start[rnti] == 0)  // no previous burst active
			|| ((rnti_burst_end[rnti] + burst_break_inactivity_ms) < current_time)) {  // last data grant was a long time ago...new burst
				rnti_last_burst_end[rnti] = rnti_burst_end[rnti];  // store end time of last burst
				rnti_burst_start[rnti] = current_time;  // start a new burst
			}
			rnti_burst_end[rnti] = current_time;
		}

		rnti_last_seen[rnti] = current_time;
	}
}

unsigned int UserActivityAnalyzer::check_rntis_inactive_set_values(
		uint64_t current_time) {
	unsigned int rntis_got_inactive = 0;

	if (current_time >= inactivity_time_ms) {  //RNTIs can only get inactive after some start period
		uint64_t threshold_time = current_time - (uint64_t) inactivity_time_ms;
		list<unsigned int>::iterator rnti_it = active_rntis.begin();
		while (rnti_it != active_rntis.end()) {
			unsigned int rnti = *rnti_it;
			if (rnti_last_seen[rnti] <= threshold_time) {  //RNTI got inactive
				if (is_c_rnti(rnti)) {  //only include c_rnti values in output data
					rntis_got_inactive++;

					uint64_t active_time = current_time - rnti_start_time[rnti]
							- inactivity_time_ms;  // define active time as time between first and last DCI for this RNTI
					if (rnti_last_burst_end[rnti] != 0) {  // we had (at least) two data bursts
						//TODO parameter
						//TODO enable/disable in settings (maybe same parameter, just set to 0)
						if ((rnti_burst_end[rnti] - rnti_burst_start[rnti])
								< network_disconnect_max_burst_size) {  // and the last burst was really short -> likely the last burst was just a disconnect message from the network
							active_time = rnti_last_burst_end[rnti] - rnti_start_time[rnti];
						}
					}

					classify_value_and_return(classes_active_time, active_time,
							active_time_values_start);
					classify_value_and_return(classes_transmitted_dl_bytes,
							rnti_bits_transmitted_dl[rnti] / 8,
							transmitted_dl_bytes_values_start);
					classify_value_and_return(classes_transmitted_ul_bytes,
							rnti_bits_transmitted_ul[rnti] / 8,
							transmitted_ul_bytes_values_start);
					if (verbose_text_output) {
						cout << "--- RNTI got inactive: " << rnti << ", active time: "
								<< active_time << "ms, dl data: "
								<< rnti_bits_transmitted_dl[rnti] / 8 << "bytes, ul data: "
								<< rnti_bits_transmitted_ul[rnti] / 8 << "bytes" << endl;
					}
				}
				rnti_start_time[rnti] = 0;  //mark as inactive
				rnti_it = active_rntis.erase(rnti_it);  //remove RNTI from active list
			} else {
				rnti_it++;
			}
		}
	}
	return rntis_got_inactive;
}

unsigned int UserActivityAnalyzer::process_rnti_active_queue(
		uint64_t current_time) {
	unsigned int rntis_got_active = 0;
	if (current_time < inactivity_time_ms) {  //RNTIs can only get (delayed) active after some start period
		return 0;
	}
	uint64_t threshold_time = current_time - inactivity_time_ms;

	while (!rnti_got_active_events.empty()) {
		rnti_got_active_event_t event = rnti_got_active_events.front();
		if (event.timestamp <= threshold_time) {
			rnti_got_active_events.pop_front();
			rntis_got_active++;
		} else {
			break;
		}
	}
	return rntis_got_active;
}

bool UserActivityAnalyzer::analyze_subframe(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	//clear values (they will be summed up/averaged in the writers)
	clear_values();

	uint64_t current_time =
			(((uint64_t) pdcch_dump_record_reader->get_sfn_iteration()) * 10240)
					+ ((uint64_t) dci_record->get_sfn() * 10)
					+ ((uint64_t) dci_record->get_subframe());

	// evaluate received DCIs, for new RNTIs and consumed data
	evaluate_dcis(current_time, dci_record, pdcch_dump_record_reader);

	// check if an RNTI active event should be added to the counters (delay)
	unsigned int rntis_got_active = process_rnti_active_queue(current_time);
	num_active_rntis += rntis_got_active;

	// check if RNTIs got inactive, update output values of data consumption and active time
	unsigned int rntis_got_inactive = check_rntis_inactive_set_values(
			current_time);
	num_active_rntis -= rntis_got_inactive;

	values[0] = num_active_rntis;
	values[1] = rntis_got_active;
	values[2] = rntis_got_inactive;

	return true;
}
