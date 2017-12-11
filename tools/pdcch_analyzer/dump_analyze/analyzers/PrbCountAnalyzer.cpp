/*
 PrbCountAnalyzer.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PrbCountAnalyzer.h"
#include "pdcch_decoder/DciResult.h"

#include <iostream>

using namespace std;

PrbCountAnalyzer::PrbCountAnalyzer() {
	//initialize value field names (used by writers)
	value_names.push_back("total allocated RBs");
	value_names.push_back("paging allocated RBs");
	value_names.push_back("UE allocated RBs");

	values.resize(3);

	set_num_samples(1); //we always provide 1 "sample" per subframe
}

PrbCountAnalyzer::~PrbCountAnalyzer() {
}

bool PrbCountAnalyzer::analyze_subframe(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	unsigned int subframe_rbs = 0;
	values[0] = 0;
	values[1] = 0;
	values[2] = 0;
	for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
			it != dci_record->get_dcis()->end(); it++) {
		DciResult* dci_result = *it;

		/* get (downlink) grant encoded in DCI and add to counters */
		srslte_ra_dl_grant_t* dl_grant = dci_result->get_dl_grant();
		if (dl_grant != 0) {
			unsigned int rbs = 0;
			rbs = dl_grant->nof_prb;
			values[0] += rbs;
			subframe_rbs += rbs;
			if (dci_result->get_rnti() == 0xFFFE) {  // paging statistics
				values[1] += rbs;
			}
			if (dci_result->get_rnti() == dci_record->get_ue_crnti()) {  // UE statistics
				values[2] += rbs;
			}
		}
	}
	if (subframe_rbs > dci_record->get_num_rbs()) {  // if the grants sum up to more than we have, there is something wrong..
		cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
		cout << "warning: more RBs allocated (" << subframe_rbs
				<< ") than available (" << dci_record->get_num_rbs() << ")" << endl;
	}
	return true;
}
