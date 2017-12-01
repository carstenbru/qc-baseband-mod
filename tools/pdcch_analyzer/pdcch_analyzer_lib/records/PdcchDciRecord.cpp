/*
 PdcchDciRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDciRecord.h"

#include <cstring>

using namespace std;

#define PDCCH_DCI_RECORD_VERSION 1
#define MAX_DCI_BYTE_LENGTH 17

//TODO compress payload data when writing/reading to file, most of the time less bytes needed

PdcchDciRecord::PdcchDciRecord(unsigned int record_version, char* data,
		unsigned int length) :
		PdcchDataRecord(PDCCH_DCI_RECORD_VERSION, data, length), dcis_in_data_bytes(
				true), dcis(0) {
}

int calc_data_size(int num_dcis) {
	return 8 + 2 + (num_dcis * MAX_DCI_BYTE_LENGTH);
}

PdcchDciRecord::PdcchDciRecord(PdcchLlrBufferRecord& pdcch_llr_record,
		std::list<DciResult*>* decoded_dcis) :
		PdcchDataRecord(PDCCH_DCI_RECORD_VERSION,
				new char[calc_data_size(decoded_dcis->size())],
				calc_data_size(decoded_dcis->size())), dcis_in_data_bytes(false), dcis(
				decoded_dcis) {
	memcpy(data, pdcch_llr_record.data, 8);
	*((uint8_t*) (data + 8)) = decoded_dcis->size();

	dcis = decoded_dcis;
}

PdcchDciRecord::~PdcchDciRecord() {
	if (dcis != 0) {
		for (list<DciResult*>::iterator it = dcis->begin(); it != dcis->end();
				it++) {
			delete *it;
		}

		delete dcis;
	}
}

string PdcchDciRecord::to_string() {
	char buf[256];
	snprintf(buf, sizeof(buf),
			"PdcchDciRecord: phy cell ID: %d\tRBs: %d\tsfn: %04d\tsubframe: %d\tCFI: %d\tDCIs: %d",
			get_phy_cell_id(), get_num_rbs(), get_sfn(), get_subframe(), get_cfi(),
			get_num_dcis());

	return string(buf);
}

list<DciResult*>* PdcchDciRecord::get_dcis() {
	if (dcis != 0) {
		return dcis;
	}

	dcis = new list<DciResult*>;
	unsigned int num_dcis = get_num_dcis();
	unsigned int tx_ports = 0;
	if (num_dcis > 0) {
		tx_ports = *((uint8_t*) (data + 9));
	}
	char* dci_src_pos = data + 10;
	for (unsigned int pos = 0; pos < num_dcis; pos++) {
		char* dci_src = dci_src_pos;
		bool prob_eq_1 = *((uint8_t*) (dci_src + 4)) >> 7;
		float prob = 1.0f;
		dci_src_pos += 5;
		if (!prob_eq_1) {  //read probability if not equal to 1
			prob = *((float*) (dci_src_pos));
			dci_src_pos += 4;
		}
		unsigned int payload_length = (*((uint8_t*) (dci_src + 1)) >> 2);
		unsigned int payload_bytes =
				(payload_length % 8 == 0) ?
						(payload_length / 8) : (payload_length / 8 + 1);
		uint64_t payload = 0;
		unsigned int shift = 0;
		while (payload_bytes--) {
			payload |= ((uint64_t)*((uint8_t*) dci_src_pos)) << shift;
			shift += 8;
			dci_src_pos++;
		}
		DciResult* dci_result = new DciResult(
				(srslte_dci_format_t) (*((uint8_t*) dci_src) & 0x1F), payload,
				payload_length, *((uint16_t*) (dci_src + 2)), get_num_rbs(), tx_ports,
				(*((uint8_t*) dci_src) >> 5));
		dci_result->set_agl_from_idx(*(dci_src + 1) & 0x3);
		dci_result->set_start_cce(*((uint8_t*) (dci_src + 4)) & 0x7F);
		dci_result->set_decoding_success_prob(prob);

		dcis->push_back(dci_result);
	}
	return dcis;
}

int PdcchDciRecord::get_data_length() {
	get_data();  //store data into byte buffer to obtain correct length
	return PdcchDumpRecord::get_data_length();
}

char* PdcchDciRecord::get_data() {
	if (!dcis_in_data_bytes) {
		char* pos = data + 10;
		if (get_num_dcis() > 0) {
			*((uint8_t*) (data + 9)) = dcis->front()->get_tx_ports();
		}
		for (list<DciResult*>::iterator it = dcis->begin(); it != dcis->end();
				it++) {
			DciResult* dci_result = *it;
			unsigned int payload_length = dci_result->get_payload_length();
			unsigned int payload_bytes =
					(payload_length % 8 == 0) ?
							(payload_length / 8) : (payload_length / 8 + 1);
			bool prob_eq_1 = (dci_result->get_decoding_success_prob() == 1.0f);

			*(pos) = dci_result->get_format() | (dci_result->get_harq_pid() << 5);
			*(pos + 1) = dci_result->get_agl_idx() | (payload_length << 2);
			*((uint16_t*) (pos + 2)) = dci_result->get_rnti();
			*(pos + 4) = dci_result->get_start_cce() | (prob_eq_1 << 7);
			pos += 5;
			if (!prob_eq_1) {  //only store probability if not equal to 1
				*((float*) pos) = dci_result->get_decoding_success_prob();
				pos += 4;
			}
			uint64_t payload = dci_result->get_payload();
			while (payload_bytes--) {  //store all used payload bytes
				*pos = payload & 0xFF;
				payload >>= 8;
				pos++;
			}
		}
		length = (unsigned int) (pos - data);
	}
	dcis_in_data_bytes = true;
	return data;
}
