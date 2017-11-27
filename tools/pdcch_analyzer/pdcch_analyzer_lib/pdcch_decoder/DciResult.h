/*
 DciResult.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef DCIRESULT_H_
#define DCIRESULT_H_

extern "C" {
#include "srsLTE/dci.h"
}

typedef unsigned long uint64_t;
typedef unsigned short uint16_t;

class DciResult {
public:
	DciResult(srslte_dci_format_t format, uint64_t payload, unsigned int length,
			uint16_t rnti, unsigned int num_rbs, unsigned int tx_ports,
			unsigned int harq_pid);
	virtual ~DciResult();

	void set_agl(unsigned int agl) {
		this->agl = agl;
	}
	void set_agl_from_idx(unsigned int agl_idx) {
		this->agl = (1 << agl_idx);
	}
	void set_start_cce(unsigned int start_cce) {
		this->start_cce = start_cce;
	}
	void set_decoding_success_prob(float decoding_success_prob) {
		this->decoding_success_prob = decoding_success_prob;
	}

	srslte_dci_format_t get_format() {
		return format;
	}
	uint64_t get_payload() {
		return payload;
	}
	unsigned int get_payload_length() {
		return length;
	}
	uint16_t get_rnti() {
		return rnti;
	}
	unsigned int get_agl() {
		return agl;
	}
	unsigned int get_agl_idx() {
		unsigned int idx = agl / 2;
		return (idx >= 4) ? 3 : idx;
	}
	unsigned int get_start_cce() {
		return start_cce;
	}
	float get_decoding_success_prob() {
		return decoding_success_prob;
	}
	char* get_format_as_string() {
		return srslte_dci_format_string(format);
	}
	unsigned int get_harq_pid() {
		return harq_pid;
	}
	unsigned int get_tx_ports() {
		return tx_ports;
	}

	bool has_ul_grant() {
		return ((format == SRSLTE_DCI_FORMAT0)
				|| ((format == SRSLTE_DCI_FORMAT0_2CQI)));
	}
	bool has_dl_grant() {
		return ((format != SRSLTE_DCI_FORMAT0) && (format != SRSLTE_DCI_FORMAT0_2CQI));
	}

	srslte_ra_dl_grant_t* get_dl_grant();
	srslte_ra_ul_grant_t* get_ul_grant();
	srslte_ra_ul_dci_t* get_ul_dec_dci();
private:
	srslte_dci_format_t format;
	uint64_t payload;
	unsigned int length;
	uint16_t rnti;
	unsigned int num_rbs;
	unsigned int tx_ports;
	unsigned int harq_pid;

	unsigned int agl;
	unsigned int start_cce;
	float decoding_success_prob;

	bool dl_grant_computed;
	srslte_ra_dl_dci_t* dl_dci;
	srslte_ra_dl_grant_t* dl_grant;

	bool ul_grant_computed;
	srslte_ra_ul_dci_t* ul_dci;
	srslte_ra_ul_grant_t* ul_grant;
};

#endif /* DCIRESULT_H_ */
