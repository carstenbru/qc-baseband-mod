/*
 DciResult.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "DciResult.h"

DciResult::DciResult(srslte_dci_format_t format, uint64_t payload,
		unsigned int length, uint16_t rnti, unsigned int num_rbs,
		unsigned int tx_ports, unsigned int harq_pid) :
		format(format), payload(payload), length(length), rnti(rnti), num_rbs(
				num_rbs), tx_ports(tx_ports), harq_pid(harq_pid), agl(0), start_cce(0), decoding_success_prob(
				0), dl_grant_computed(false), dl_dci(0), dl_grant(0), ul_grant_computed(
				false), ul_dci(0), ul_grant(0) {

}

DciResult::~DciResult() {
	if (dl_grant_computed) {
		delete dl_dci;
		delete dl_grant;
	}
	if (ul_grant_computed) {
		delete ul_dci;
		delete ul_grant;
	}
}

srslte_ra_dl_grant_t* DciResult::get_dl_grant() {
	if (!has_dl_grant()) {
		return 0;
	}
	if (!dl_grant_computed) {
		dl_dci = new srslte_ra_dl_dci_t;
		dl_grant = new srslte_ra_dl_grant_t;
		srslte_dci_msg_t msg;
		for (unsigned int i = 0; i < length; i++) {
			msg.data[i] = (payload >> i) & 1;
		}
		msg.nof_bits = length;
		msg.format = format;
		srslte_dci_msg_to_dl_grant(&msg, rnti, num_rbs, tx_ports, dl_dci, dl_grant);
		dl_grant_computed = true;
	}
	return dl_grant;
}

srslte_ra_ul_grant_t* DciResult::get_ul_grant() {
	if (!has_ul_grant()) {
		return 0;
	}
	if (!ul_grant_computed) {
		ul_dci = new srslte_ra_ul_dci_t;
		ul_grant = new srslte_ra_ul_grant_t;
		srslte_dci_msg_t msg;
		for (unsigned int i = 0; i < length; i++) {
			msg.data[i] = (payload >> i) & 1;
		}
		msg.nof_bits = length;
		msg.format = format;
		srslte_dci_msg_to_ul_grant(&msg, num_rbs, 0, ul_dci, ul_grant, harq_pid);
		ul_grant_computed = true;
	}
	return ul_grant;
}

srslte_ra_ul_dci_t* DciResult::get_ul_dec_dci() {
	if (!has_ul_grant()) {
		return 0;
	}
	if (!ul_grant_computed) {
		get_ul_grant();
	}
	return ul_dci;
}
