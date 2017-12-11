/*
 UlMcsAnalyzer.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "UlMcsAnalyzer.h"

using namespace std;

#include <iostream>
UlMcsAnalyzer::UlMcsAnalyzer() :
		exclude_cqi_request_only(true), exclude_retransmission_mcs(false), exclude_own_rnti(
				true) {
	update_fields();
}

UlMcsAnalyzer::~UlMcsAnalyzer() {
}

bool UlMcsAnalyzer::set_parameter(string name, vector<string>& values) {
	if (SubframeAnalyzer::set_parameter(name, values)) {
		return true;
	}
	if (name.compare("exclude_cqi_request_only") == 0) {
		int int_val = atoi(values[1].c_str());
		set_exclude_cqi_request_only(int_val);
	} else if (name.compare("exclude_retransmission_mcs") == 0) {
		int int_val = atoi(values[1].c_str());
		set_exclude_retransmission_mcs(int_val);
	} else if (name.compare("exclude_own_rnti") == 0) {
		int int_val = atoi(values[1].c_str());
		set_exclude_own_rnti(int_val);
	}

	return false;
}

void UlMcsAnalyzer::update_fields() {
	unsigned int num_mcs =
			(exclude_cqi_request_only && exclude_retransmission_mcs) ? 29 : 32;
	value_names.clear();

	//initialize value field names (used by writers)
	char buf[100];
	for (unsigned int mcs_index = 0; mcs_index < num_mcs; mcs_index++) {
		snprintf(buf, 100, "UL MCS %d", mcs_index);
		value_names.push_back(buf);
	}

	values.resize(num_mcs);
}

void UlMcsAnalyzer::set_exclude_cqi_request_only(
		bool exclude_cqi_request_only) {
	this->exclude_cqi_request_only = exclude_cqi_request_only;
	update_fields();
}
void UlMcsAnalyzer::set_exclude_retransmission_mcs(
		bool exclude_retransmission_mcs) {
	this->exclude_retransmission_mcs = exclude_retransmission_mcs;
	update_fields();
}

bool UlMcsAnalyzer::analyze_subframe(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	//clear values (they will be summed up/averaged in the writers)
	for (unsigned int mcs_index = 0; mcs_index < 32; mcs_index++) {
		values[mcs_index] = 0;
	}
	unsigned int num_samples = 0;

	for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
			it != dci_record->get_dcis()->end(); it++) {
		DciResult* dci_result = *it;

		/* get (uplink) grant encoded in DCI and get MCS */
		srslte_ra_ul_grant_t* ul_grant = dci_result->get_ul_grant();
		if (ul_grant != 0) {  //TODO how to deal with UE supporting 64-QAM or not? MCS has a different meaning then and is not comparable
			bool cqi_only_req = false;
			bool retx = false;
			bool exclude = false;
			if (exclude_cqi_request_only || exclude_retransmission_mcs) {
				if (ul_grant->mcs.idx == 29) {
					srslte_ra_ul_dci_t* ul_dec_dci = dci_result->get_ul_dec_dci();
					//TODO implement clean, according to 36.213 8.6.2
					if ((ul_dec_dci->cqi_request) && (ul_grant->L_prb <= 4)) {
						cqi_only_req = true;
					} else if ((ul_dec_dci->cqi_request_2) && (ul_grant->L_prb <= 20)) {
						cqi_only_req = true;
					}
				}
				retx = (ul_grant->mcs.idx >= 29) && !cqi_only_req;
			}
			if (exclude_own_rnti) {
				if (dci_result->get_rnti() == dci_record->get_ue_crnti()) {
					exclude = true;
				}
			}

			if (!(exclude_cqi_request_only && cqi_only_req)
					&& !(exclude_retransmission_mcs && retx) && !exclude) {
				values[ul_grant->mcs.idx]++;
				num_samples++;
			}
		}
	}
	set_num_samples(num_samples);
	return true;
}
