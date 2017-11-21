/*
 PdcchDciRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDciRecord.h"

#include <cstring>

using namespace std;

#define PDCCH_DCI_RECORD_VERSION 0
#define DCI_BYTE_LENGTH 17

//TODO compress payload data when writing/reading to file, most of the time less bytes needed

PdcchDciRecord::PdcchDciRecord(unsigned int record_version, char* data,
		unsigned int length) :
		PdcchDataRecord(PDCCH_DCI_RECORD_VERSION, data, length), dcis_in_data_bytes(
				true), dcis(0) {
}

int calc_data_size(int num_dcis) {
	return 8 + 2 + (num_dcis * DCI_BYTE_LENGTH);
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
	for (unsigned int pos = 0; pos < num_dcis; pos++) {
		char* dci_src = data + 10 + pos * DCI_BYTE_LENGTH;
		DciResult* dci_result = new DciResult((srslte_dci_format_t)(*((uint8_t*)dci_src) & 0x1F),
				*((uint64_t*) (dci_src + 4)), (*((uint8_t*)(dci_src + 1)) >> 2),
				*((uint16_t*) (dci_src + 2)), get_num_rbs(), tx_ports,
				(*((uint8_t*)dci_src) >> 5));
		dci_result->set_agl_from_idx(*(dci_src + 1) & 0x3);
		dci_result->set_start_cce(*(dci_src + 16));
		dci_result->set_decoding_success_prob(*((float*) (dci_src + 12)));

		dcis->push_back(dci_result);
	}
	return dcis;
}

char* PdcchDciRecord::get_data() {
	if (!dcis_in_data_bytes) {
		int pos = 0;
		if (get_num_dcis() > 0) {
			*((uint8_t*) (data + 9)) = dcis->front()->get_tx_ports();
		}
		for (list<DciResult*>::iterator it = dcis->begin(); it != dcis->end();
				it++) {
			DciResult* dci_result = *it;
			char* dci_dest = data + 10 + pos * DCI_BYTE_LENGTH;

			*(dci_dest) = dci_result->get_format()
					| (dci_result->get_harq_pid() << 5);
			*(dci_dest + 1) = dci_result->get_agl_idx()
					| (dci_result->get_payload_length() << 2);
			*((uint16_t*) (dci_dest + 2)) = dci_result->get_rnti();
			*((uint64_t*) (dci_dest + 4)) = dci_result->get_payload();
			*((float*) (dci_dest + 12)) = dci_result->get_decoding_success_prob();
			*(dci_dest + 16) = dci_result->get_start_cce();

			pos++;
		}
	}
	dcis_in_data_bytes = true;
	return data;
}
