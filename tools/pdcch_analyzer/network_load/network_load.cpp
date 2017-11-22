/*
 network_load.cpp

 Carsten Bruns (carst.bruns@gmx.de)
 */

#include <pdcch_decoder/DciResult.h>
#include <pdcch_decoder/srsLTE/phy_common.h>
#include <pdcch_decoder/srsLTE/ra.h>
#include <PdcchDumpRecordReader.h>
#include <records/PdcchDciRecord.h>
#include <records/PdcchTimeRecord.h>

#include <string.h>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

using namespace std;

ofstream it_sfn_file_stream;
ofstream sfn_file_stream;

unsigned int rnti_count[65536];

/*
 * write headers in output files if it is the first timestamp (the output only starts then)
 */
bool process_timestamp_record(PdcchTimeRecord* time_record, void* arg) {
	PdcchDumpRecordReader* pdcch_dump_record_reader = (PdcchDumpRecordReader*) arg;
	if (pdcch_dump_record_reader->get_last_record(PDCCH_TIME_RECORD) == 0) {  //first time record
		sfn_file_stream << "#" << time_record->getTimeString() << "\n";
		it_sfn_file_stream << "#" << time_record->getTimeString() << "\n";

		sfn_file_stream << "SFN iteration" << "\t" << "SFN" << "\t"
				<< "cell data rate [kbit/s]" << "\t" << "paging data rate [kbit/s]"
				<< "\t" << "UE data rate [kbit/s]" << "\t" << "total allocated RBs"
				<< "\t" << "paging allocated RBs" << "\t" << "UE allocated RBs" << "\n";
		it_sfn_file_stream << "Timestamp" << "\t" << "SFN iteration" << "\t"
				<< "cell data rate [kbit/s]" << "\t" << "paging data rate [kbit/s]"
				<< "\t" << "UE data rate [kbit/s]" << "\t" << "total allocated RBs"
				<< "\t" << "paging allocated RBs" << "\t" << "UE allocated RBs" << "\n";
	}
	return true;
}

bool dci_callback_load_data_rate(PdcchDciRecord* dci_record, void* arg) {
	static int samples[] = { 0, 0 };
	static unsigned long int data_rate[] = { 0, 0 };
	static unsigned long int paging_data_rate[] = { 0, 0 };
	static unsigned long int ue_data_rate[] = { 0, 0 };
	static unsigned long int allocated_rbs[] = { 0, 0 };
	static unsigned long int paging_allocated_rbs[] = { 0, 0 };
	static unsigned long int ue_allocated_rbs[] = { 0, 0 };

	PdcchDumpRecordReader* pdcch_dump_record_reader = (PdcchDumpRecordReader*) arg;

	/* collect statistics */
	if (pdcch_dump_record_reader->get_last_record(PDCCH_TIME_RECORD) != 0) {  // only write if we have seen a timestamp
		unsigned int subframe_rbs = 0;
		for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
				it != dci_record->get_dcis()->end(); it++) {
			DciResult* dci_result = *it;
			rnti_count[dci_result->get_rnti()]++;

			/* get (downlink) grant encoded in DCI and add to counters */
			srslte_ra_dl_grant_t* dl_grant = dci_result->get_dl_grant();
			if (dl_grant != 0) {
				unsigned int rbs = 0;
				rbs = dl_grant->nof_prb;
				unsigned int d_rate = 0;
				for (int i = 0; i < SRSLTE_MAX_CODEWORDS; i++) {
					if (dl_grant->tb_en[i]) {
						if (dl_grant->mcs[i].tbs != -1) {
							d_rate += dl_grant->mcs[i].tbs;
						}
					}
				}
				data_rate[0] += d_rate;
				allocated_rbs[0] += rbs;
				subframe_rbs += rbs;
				if (dci_result->get_rnti() == 0xFFFE) {  // paging statistics
					paging_data_rate[0] += d_rate;
					paging_allocated_rbs[0] += rbs;
				}
				if (dci_result->get_rnti() == dci_record->get_ue_crnti()) {  // UE statistics
					ue_data_rate[0] += d_rate;
					ue_allocated_rbs[0] += rbs;
				}
			}
		}
		if (subframe_rbs > dci_record->get_num_rbs()) {  // if the grants sum up to more than we have, there is something wrong..
			cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
			cout << "warning: more RBs allocated (" << subframe_rbs
					<< ") than available (" << dci_record->get_num_rbs() << ")" << endl;
		}

		/* output writing */
		if (pdcch_dump_record_reader->get_last_sfn() > dci_record->get_sfn()) {  // once per SFN iteration (i.e. every 1024th frame)
			if (samples[1] > 100) {  // only if we have enough samples
				it_sfn_file_stream
						<< pdcch_dump_record_reader->get_time_string(dci_record) << "\t"
						<< pdcch_dump_record_reader->get_sfn_iteration() << "\t"
						<< data_rate[1] / (float) samples[1] << "\t"
						<< paging_data_rate[1] / (float) samples[1] << "\t"
						<< ue_data_rate[1] / (float) samples[1] << "\t"
						<< allocated_rbs[1] / (float) samples[1] << "\t"
						<< paging_allocated_rbs[1] / (float) samples[1] << "\t"
						<< ue_allocated_rbs[1] / (float) samples[1] << "\n";
			}

			// reset all counters
			data_rate[1] = 0;
			paging_data_rate[1] = 0;
			ue_data_rate[1] = 0;
			allocated_rbs[1] = 0;
			paging_allocated_rbs[1] = 0;
			ue_allocated_rbs[1] = 0;

			samples[1] = 0;
		}
		samples[0]++;
		if (dci_record->get_subframe() == 9) {  // once per SFN
			// copy to counters for SFN iteration statistics
			data_rate[1] += data_rate[0];
			paging_data_rate[1] += paging_data_rate[0];
			ue_data_rate[1] += ue_data_rate[0];
			allocated_rbs[1] += allocated_rbs[0];
			paging_allocated_rbs[1] += paging_allocated_rbs[0];
			ue_allocated_rbs[1] += ue_allocated_rbs[0];

			sfn_file_stream << pdcch_dump_record_reader->get_sfn_iteration() << "\t"
					<< dci_record->get_sfn() << "\t" << data_rate[0] / (float) samples[0]
					<< "\t" << paging_data_rate[0] / (float) samples[0] << "\t"
					<< ue_data_rate[0] / (float) samples[0] << "\t"
					<< allocated_rbs[0] / (float) samples[0] << "\t"
					<< paging_allocated_rbs[0] / (float) samples[0] << "\t"
					<< ue_allocated_rbs[0] / (float) samples[0] << "\n";

			// reset all counters
			data_rate[0] = 0;
			paging_data_rate[0] = 0;
			ue_data_rate[0] = 0;
			allocated_rbs[0] = 0;
			paging_allocated_rbs[0] = 0;
			ue_allocated_rbs[0] = 0;

			samples[1] += samples[0];
			samples[0] = 0;
		}
		cout << "it: " << pdcch_dump_record_reader->get_sfn_iteration() << " sfn: "
				<< dci_record->get_sfn() << " subframe: " << dci_record->get_subframe()
				<< endl;
	}

	return false;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: " << argv[0] << " DUMP_FILE_PATH DUMP_NAME" << endl;
		return 0;
	}

	/* open source and target files */
	string it_sfn_filename = "./it_sfn_";
	it_sfn_filename.append(argv[2]);
	it_sfn_filename.append(".csv");
	it_sfn_file_stream.open(it_sfn_filename);
	string sfn_filename = "./sfn_";
	sfn_filename.append(argv[2]);
	sfn_filename.append(".csv");
	sfn_file_stream.open(sfn_filename);

	string filename = argv[1];
	filename.append("/");
	filename.append(argv[2]);
	filename.append(".bin");
	memset(rnti_count, 0, sizeof(unsigned int) * 65536);

	/* create record reader, activate decoder and register callbacks */
	PdcchDumpRecordReader pdcch_dump_record_reader(filename, true);
	pdcch_dump_record_reader.register_callback(PDCCH_TIME_RECORD,
			(record_callback_t) &process_timestamp_record, &pdcch_dump_record_reader);
	pdcch_dump_record_reader.register_callback(PDCCH_DCI_RECORD,
			(record_callback_t) &dci_callback_load_data_rate,
			&pdcch_dump_record_reader);

	/* let's go! */
	pdcch_dump_record_reader.read_all_records();

	cout << "finished processing dump" << endl;

	/* output list of seen RNTIs */
	for (unsigned int i = 0; i < 65536; i++) {
		if (rnti_count[i] > 0) {
			cout << "RNTI " << i << ": " << rnti_count[i] << endl;
		}
	}

	return 0;
}

