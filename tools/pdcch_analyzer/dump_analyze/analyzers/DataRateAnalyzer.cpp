/*
 DataRateAnalyzer.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "DataRateAnalyzer.h"

#include <iostream>

using namespace std;

DataRateAnalyzer::DataRateAnalyzer() {
	//initialize value field names (used by writers)
	value_names.push_back("cell data rate [kbit/s]");
	value_names.push_back("paging data rate [kbit/s]");
	value_names.push_back("UE data rate [kbit/s]");

	values.resize(3);

	set_num_samples(1); //we always provide 1 "sample" per subframe
}

DataRateAnalyzer::~DataRateAnalyzer() {
}

bool DataRateAnalyzer::analyze_subframe(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	values[0] = 0;
	values[1] = 0;
	values[2] = 0;
	for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
			it != dci_record->get_dcis()->end(); it++) {
		DciResult* dci_result = *it;

		/* get (downlink) grant encoded in DCI and add to counters */
		srslte_ra_dl_grant_t* dl_grant = dci_result->get_dl_grant();
		if (dl_grant != 0) {
			unsigned int d_rate = 0;
			for (int i = 0; i < SRSLTE_MAX_CODEWORDS; i++) {
				if (dl_grant->tb_en[i]) {
					if (dl_grant->mcs[i].tbs != -1) {
						d_rate += dl_grant->mcs[i].tbs;
					}
				}
			}
			values[0] += d_rate;
			if (dci_result->get_rnti() == 0xFFFE) {  // paging statistics
				values[1] += d_rate;
			}
			if (dci_result->get_rnti() == dci_record->get_ue_crnti()) {  // UE statistics
				values[2] += d_rate;
			}
		}
	}
	return true;
}
