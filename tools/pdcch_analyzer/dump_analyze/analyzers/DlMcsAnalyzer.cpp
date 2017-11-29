/*
 DlMcsAnalyzer.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "DlMcsAnalyzer.h"

using namespace std;

#include <iostream>
DlMcsAnalyzer::DlMcsAnalyzer() :
		exclude_special_rntis(true), exclude_own_rnti(true) {
	//initialize value field names (used by writers)
	char buf[100];
	for (unsigned int mcs_index = 0; mcs_index < 32; mcs_index++) {
		snprintf(buf, 100, "DL MCS %d", mcs_index);
		value_names.push_back(buf);
	}

	values.resize(32);
}

DlMcsAnalyzer::~DlMcsAnalyzer() {
}

bool DlMcsAnalyzer::set_parameter(string name, string value) {
	if (SubframeAnalyzer::set_parameter(name, value)) {
		return true;
	}
	if (name.compare("exclude_special_rntis") == 0) {
		int int_val = atoi(value.c_str());
		set_exclude_special_rntis(int_val);
	} else if (name.compare("exclude_own_rnti") == 0) {
		int int_val = atoi(value.c_str());
		set_exclude_own_rnti(int_val);
	}

	return false;
}

bool DlMcsAnalyzer::analyze_subframe(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	//clear values (they will be summed up/averaged in the writers)
	for (unsigned int mcs_index = 0; mcs_index < 32; mcs_index++) {
		values[mcs_index] = 0;
	}
	num_samples = 0;

	for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
			it != dci_record->get_dcis()->end(); it++) {
		DciResult* dci_result = *it;

		/* get (downlink) grant encoded in DCI and get MCS */
		srslte_ra_dl_grant_t* dl_grant = dci_result->get_dl_grant();
		if (dl_grant != 0) {  //TODO how to deal with new (256-QAM supported) devices? MCS has a different meaning then and is not comparable
			for (int i = 0; i < SRSLTE_MAX_CODEWORDS; i++) {  //TODO how to deal with two transport blocks with different/same MCS? add both to distribution?
				if (dl_grant->tb_en[i]) {
					bool exclude = false;
					if (exclude_special_rntis) {
						if ((dci_result->get_rnti() < 0x0001)
								|| (dci_result->get_rnti() > 0xFFF3)) {
							exclude = true;
						}
					}
					if (exclude_own_rnti) {
						if (dci_result->get_rnti() == dci_record->get_ue_crnti()) {
							exclude = true;
						}
					}

					if (!exclude) {
						values[dl_grant->mcs[i].idx]++;
						num_samples++;
					}
				}
			}
		}
	}
	return true;
}
