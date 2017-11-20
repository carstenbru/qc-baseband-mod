/*
 PdcchDecoder.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDecoder.h"

#include <iostream>
#include <string>
#include <cstring>

extern "C" {
#include "srsLTE/crc.h"
#include "srsLTE/rm_conv.h"
#include "srsLTE/viterbi.h"
#include "srsLTE/bit.h"
#include "srsLTE/convcoder.h"
}

using namespace std;

#define DEFAULT_REG_ENERGY_THRESHOLD 128
#define DEFAULT_DECODE_SUCCESS_PROB_THRESHOLD 0.99f
#define DEFAULT_MIN_REGS_W_ENEGRY_CCE_ACTIVE 5

#define PDCCH_NCOLS  32
const uint8_t PDCCH_PERM[PDCCH_NCOLS] = { 1, 17, 9, 25, 5, 21, 13, 29, 3, 19,
		11, 27, 7, 23, 15, 31, 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22,
		14, 30 };
#define Nc 1600
#define SRSLTE_LTE_CRC16   0x11021

//TODO dynamically update
srslte_dci_format_t active_dci_formats[] = { SRSLTE_DCI_FORMAT1C,
		SRSLTE_DCI_FORMAT0, SRSLTE_DCI_FORMAT0_SRS, SRSLTE_DCI_FORMAT1B,
		SRSLTE_DCI_FORMAT1, SRSLTE_DCI_FORMAT2A };  //, SRSLTE_DCI_FORMAT2 };  //SRSLTE_DCI_FORMAT1A is activated with SRSLTE_DCI_FORMAT0 as they always have the same length
//ordered by increasing length, as for same probabilities the first result is kept and mistakes are less likely for shorter DCI types (more bits for error correction)

PdcchDecoder::PdcchDecoder() :
		decode_success_prob_threshold(DEFAULT_DECODE_SUCCESS_PROB_THRESHOLD), pdcch_add_cell_info_record(
				0), last_phy_cell_id(-1), num_dci_formats(0), reg_energy_threshold(
		DEFAULT_REG_ENERGY_THRESHOLD) {
	for (unsigned int i = 0; i < 3; i++) {
		cce_reg_to_reg_pos_mapping[i] = 0;
	}
	for (unsigned int i = 0; i < 10; i++) {
		scramble_seq[i] = 0;
	}
}

PdcchDecoder::~PdcchDecoder() {
}

void PdcchDecoder::register_callback(decoder_callback_t callback, void* arg) {
	callbacks.push_back(callback);
	callback_args.push_back(arg);
}

bool PdcchDecoder::new_pdcch_add_cell_info_record(
		PdcchAddCellInfoRecord* pdcch_add_cell_info_record) {
	if (this->pdcch_add_cell_info_record != 0) {
		delete this->pdcch_add_cell_info_record;
	}
	this->pdcch_add_cell_info_record = pdcch_add_cell_info_record;
	return true;
}

/**
 * calculates the resource grid positions of all REGs of PDCCH
 */
void phy_reg_to_reg_idx(unsigned int num_regs, unsigned int control_symbols,
		char* reg_ch_map, reg_idx* reg_idx_map) {
	unsigned int cur_sc = 0;
	unsigned int cur_sym_idx = 0;
	unsigned int last_sym_idx = 0;

	while (num_regs--) {
		bool valid = false;

		while (!valid) {
			//check if valid
			if (cur_sym_idx == 0) {
				if (cur_sc % 6 == 0) {  //TODO use REG_HEIGHT values from modem instead of hard-coded 4 and 6
					unsigned int reg_idx_sym0 = cur_sc / 6;
					unsigned int map_word = *((unsigned int*) (reg_ch_map
							+ reg_idx_sym0 / 32 * 4));
					if (((map_word >> (reg_idx_sym0 % 32)) & 1) == 0) {
						reg_idx_map->reg_nr = reg_idx_sym0;
						valid = true;
					}
				}
			} else {
				if (cur_sc % 4 == 0) {
					reg_idx_map->reg_nr = cur_sc >> 2;
					valid = true;
				}
			}

			//go to next possible start
			last_sym_idx = cur_sym_idx;
			cur_sym_idx++;
			if (cur_sym_idx >= control_symbols) {
				cur_sym_idx = 0;
				cur_sc += 2;
			}
		}
		reg_idx_map->sym_idx = last_sym_idx;
		reg_idx_map++;
	}
}

void calc_cce_reg_to_reg_pos_mapping(unsigned int num_regs,
		unsigned int cell_id, unsigned int num_control_symbols, char* reg_ch_map,
		reg_idx* map) {
	/* determine phy REG locations in resource grid */
	reg_idx* reg_idx_map = new reg_idx[num_regs];
	phy_reg_to_reg_idx(num_regs, num_control_symbols, reg_ch_map, reg_idx_map);

	/* calculate REG interleaving and cyclic shift according to 36.211 10.3 section 6.8.5 (adapted srsLTE code),
	 * directly map numbers to final grid locations */
	unsigned int nrows = (num_regs - 1) / PDCCH_NCOLS + 1;
	unsigned int ndummy = PDCCH_NCOLS * nrows - num_regs;
	if (ndummy < 0) {
		ndummy = 0;
	}

	unsigned int k = 0;
	unsigned int kp;
	for (unsigned int j = 0; j < PDCCH_NCOLS; j++) {
		for (unsigned int i = 0; i < nrows; i++) {
			if (i * PDCCH_NCOLS + PDCCH_PERM[j] >= ndummy) {
				unsigned int m = i * PDCCH_NCOLS + PDCCH_PERM[j] - ndummy;
				if (k < cell_id) {
					kp = (num_regs + k - (cell_id % num_regs)) % num_regs;
				} else {
					kp = (k - cell_id) % num_regs;
				}
				map[m] = reg_idx_map[kp];
				k++;
			}
		}
	}
	delete[] reg_idx_map;
}

/**
 * calculate scrambling sequence
 *
 * from srsLTE
 */
void calc_scrambling_sequence(unsigned int len, uint32_t x2_start_seed,
		uint32_t* c) {
	unsigned int n;
	uint32_t *x1, *x2;

	x1 = (uint32_t*) calloc(Nc + len + 31, sizeof(uint32_t));
	if (!x1) {
		perror("calloc");
		return;
	}
	x2 = (uint32_t*) calloc(Nc + len + 31, sizeof(uint32_t));
	if (!x2) {
		free(x1);
		perror("calloc");
		return;
	}

	for (n = 0; n < 31; n++) {
		x2[n] = (x2_start_seed >> n) & 0x1;
	}
	x1[0] = 1;

	for (n = 0; n < Nc + len; n++) {
		x1[n + 31] = (x1[n + 3] + x1[n]) & 0x1;
		x2[n + 31] = (x2[n + 3] + x2[n + 2] + +x2[n + 1] + x2[n]) & 0x1;
	}

	for (n = 0; n < len; n++) {
		c[n] = (x1[n + Nc] + x2[n + Nc]) & 0x1;
	}

	free(x1);
	free(x2);
}

void PdcchDecoder::pre_calculate_values(uint16_t phy_cell_id, unsigned int prbs,
		unsigned int tx_ports) {
	last_phy_cell_id = phy_cell_id;

	/* REG mapping (interleaving, cyclic-shift, ordering) */
	for (unsigned int cfi = 1; cfi <= 3; cfi++) {
		if (cce_reg_to_reg_pos_mapping[cfi - 1] != 0) {
			delete[] cce_reg_to_reg_pos_mapping[cfi - 1];
		}
		cce_reg_to_reg_pos_mapping[cfi - 1] =
				new reg_idx[pdcch_add_cell_info_record->get_num_regs(cfi)];

		calc_cce_reg_to_reg_pos_mapping(
				pdcch_add_cell_info_record->get_num_regs(cfi), phy_cell_id, cfi,
				pdcch_add_cell_info_record->get_reg_map(0),  //TODO function to calculate reg_map (belonging to PDCCH) as alternative
				cce_reg_to_reg_pos_mapping[cfi - 1]);
	}

	/* PDCCH scrambling sequence (for all 10 subframes) */
	unsigned int len_cfi3 = pdcch_add_cell_info_record->get_num_regs(3) * 8;
	for (unsigned int subframe = 0; subframe < 10; subframe++) {
		if (scramble_seq[subframe] != 0) {
			delete[] scramble_seq[subframe];
		}
		scramble_seq[subframe] = new uint32_t[len_cfi3];
		calc_scrambling_sequence(len_cfi3, (subframe << 9) + phy_cell_id,
				scramble_seq[subframe]);
	}

	/* DCI format lengths */
	num_dci_formats = sizeof(active_dci_formats) / sizeof(srslte_dci_format_t);
	for (unsigned int f = 0; f < num_dci_formats; f++) {
		if (active_dci_formats[f] == SRSLTE_DCI_FORMAT0_SRS) {  //TODO clean implementation
			dci_format_lengths[f] = srslte_dci_format_sizeof(SRSLTE_DCI_FORMAT0, prbs,
					tx_ports) + 1;
		} else {
			dci_format_lengths[f] = srslte_dci_format_sizeof(active_dci_formats[f],
					prbs, tx_ports);
		}
	}
}

void crc_set_mask_rnti(uint8_t *crc, uint16_t rnti) {
	uint32_t i;
	uint8_t mask[16];
	uint8_t *r = mask;

	srslte_bit_unpack(rnti, &r, 16);
	for (i = 0; i < 16; i++) {
		crc[i] = (crc[i] + mask[i]) % 2;
	}
}

float PdcchDecoder::try_decode_dci(int16_t* ch_data, unsigned int agl,
		unsigned int dci_bits, uint16_t* decoded_rnti, uint64_t* decoded_dci_bits) {
	uint8_t recovered_dci_bits[dci_bits + 16];

	unsigned int num_ch_bits = agl * 72;
	float prob = 0;

	uint16_t p_bits, crc_res, c_rnti;
	uint8_t *x;
	int16_t rm[3 * (SRSLTE_DCI_MAX_BITS + 16)];
	srslte_crc_t q_crc;
	srslte_crc_init(&q_crc, SRSLTE_LTE_CRC16, 16);

	// packet decoding
	bzero(rm, sizeof(int16_t) * 3 * (SRSLTE_DCI_MAX_BITS + 16));

	/* unrate matching */
	srslte_rm_conv_rx_s(ch_data, num_ch_bits, rm, 3 * (dci_bits + 16));

	/* viterbi decoder */
	srslte_viterbi_t decoder;
	int poly[3] = { 0x6D, 0x4F, 0x57 };
	srslte_viterbi_init(&decoder, SRSLTE_VITERBI_37, poly,
	SRSLTE_DCI_MAX_BITS + 16, true);
	srslte_viterbi_decode_s(&decoder, rm, recovered_dci_bits, dci_bits + 16);
	srslte_viterbi_free(&decoder);  //TODO use same decoder multiple times (make it a member of the decoder class)

	x = &recovered_dci_bits[dci_bits];
	p_bits = (uint16_t) srslte_bit_pack(&x, 16);
	crc_res = ((uint16_t) srslte_crc_checksum(&q_crc, recovered_dci_bits,
			dci_bits) & 0xffff);  //calculate CRC
	c_rnti = p_bits ^ crc_res;  //XOR with received last 16bit to get RNTI value
	*decoded_rnti = c_rnti;

	// #############################################

	// re-encoding the packet
	uint8_t tmp[3 * (SRSLTE_DCI_MAX_BITS + 16)];
	uint8_t tmp2[10 * (SRSLTE_DCI_MAX_BITS + 16)];
	uint8_t check[(SRSLTE_DCI_MAX_BITS + 16)];
	srslte_convcoder_t encoder;

	encoder.K = 7;
	encoder.R = 3;
	encoder.tail_biting = true;
	memcpy(encoder.poly, poly, 3 * sizeof(int));

	memcpy(check, recovered_dci_bits, dci_bits);

	srslte_crc_attach(&q_crc, check, dci_bits);
	crc_set_mask_rnti(&check[dci_bits], c_rnti);

	srslte_convcoder_encode(&encoder, check, tmp, dci_bits + 16);
	srslte_rm_conv_tx(tmp, 3 * (dci_bits + 16), tmp2, num_ch_bits);

	// probability heuristic
	unsigned int sum = 0;
	for (unsigned int i = 0; i < num_ch_bits; i++) {
		//	prob += ((ch_data[i] > 0) == tmp2[i]);

		prob += tmp2[i] ? ch_data[i] : -ch_data[i];
		sum += abs(ch_data[i]);
	}
	//prob /= num_ch_bits;
	prob /= sum;

	if (*decoded_rnti == 0) {
		prob = 0;
	} else {
		uint64_t bits = 0;
		for (unsigned int i = 0; i < dci_bits; i++) {
			bits |= ((uint64_t) (recovered_dci_bits[i] & 1)) << i;
		}
		*decoded_dci_bits = bits;
	}

	return prob;
}

bool PdcchDecoder::validate_rnti_in_search_space(unsigned int agl,
		unsigned int num_cce, unsigned int subframe, uint16_t rnti,
		unsigned int start_cce) {

	/* check if CCE is part of CSS */
	unsigned int css_size = (num_cce < 16) ? num_cce : 16;  //TODO optimize this!! can be done way faster..
	for (unsigned int i = 0; i < css_size / (agl); i++) {
		uint32_t ncce = (agl) * (i % (num_cce / (agl)));
		if (ncce + agl <= num_cce) {
			if (ncce == start_cce) {
				return true;
			}
		}
	}

	// compute Yk for this subframe
	uint32_t Yk = rnti;
	for (unsigned int m = 0; m < subframe + 1; m++) {
		Yk = (39827 * Yk) % 65537;
	}

	/* check if CCE is part of UESSS */
	unsigned int num_candidates = (agl >= 4) ? 2 : 6;
	for (unsigned int i = 0; i < num_candidates; i++) {
		if (num_cce >= agl) {
			uint32_t ncce = agl * ((Yk + i) % (num_cce / agl));
			// Check if candidate fits in CCE region
			if (ncce + agl <= num_cce) {
				if (ncce == start_cce) {
					return true;
				}
			}
		}
	}

	return false;
}

void PdcchDecoder::blind_decode(int16_t* cce_buf, unsigned int num_regs,
		PdcchLlrBufferRecord& llr_buffer_record) {
	list<DciResult*> detected_dcis;

	bool cce_dci_detected[num_regs / 9];
	for (unsigned int cce = 0; cce < num_regs / 9; cce++) {
		cce_dci_detected[cce] = false;
	}

	/* check which CCEs have "energy" */
	uint8_t active_cce[num_regs / 9];
	if (reg_energy_threshold > 0) {
		for (unsigned int cce = 0; cce < num_regs / 9; cce++) {
			unsigned int reg_w_energy = 0;
			for (unsigned int reg = 0; reg < 9; reg++) {
				unsigned int energy = 0;
				for (unsigned int bit = 0; bit < 8; bit++) {
					energy += abs(*(cce_buf + cce * 8 * 9 + reg * 8 + bit));
				}
				if (energy > reg_energy_threshold) {
					reg_w_energy++;
				}
			}
			active_cce[cce] = reg_w_energy;
		}
	}

	/* actual decoding */
	for (unsigned int agl = 8; agl >= 1; agl >>= 1) {
		for (unsigned int candidate = 0; candidate < num_regs / 9 / agl;
				candidate++) {
			//check if enough CCEs of the candidate have detected energy
			unsigned int active_cce_count = 0;
			for (unsigned int cce = candidate * agl; cce < (candidate + 1) * agl;
					cce++) {
				if (active_cce[cce] >= DEFAULT_MIN_REGS_W_ENEGRY_CCE_ACTIVE) {
					active_cce_count++;
				}
			}
			unsigned int min_active_cce = agl - (agl / 2);  //we want for AGL1: 1; AGL2: 1; AGL4: 2; AGL8: 4
			if (active_cce_count < min_active_cce) {
				continue;
			}

			float best_prob = 0;
			unsigned int best_format_idx = SRSLTE_DCI_NOF_FORMATS;
			uint16_t best_decoded_rnti = 0;
			uint64_t best_decoded_dci_bits;

			for (unsigned int dci_format = 0; dci_format < num_dci_formats;
					dci_format++) {
				if (active_dci_formats[dci_format] == SRSLTE_DCI_FORMAT1A) {
					continue;  //skip format 1A as it has same length as format 0, so we already tried to decode its DCIs
				}
				if (cce_dci_detected[candidate * agl]) {
					continue;  //skip CCEs in which already a DCI was detected
				}
				uint16_t decoded_rnti;
				uint64_t decoded_dci_bits;
				float prob = try_decode_dci(cce_buf + candidate * agl * 72, agl,
						dci_format_lengths[dci_format], &decoded_rnti, &decoded_dci_bits);

				/* checks whether P/SI/RA-RNTI addresses are used with formats other than 0 and 1A */
				if (!(active_dci_formats[dci_format] == SRSLTE_DCI_FORMAT0
						|| active_dci_formats[dci_format] == SRSLTE_DCI_FORMAT1A)
						&& (decoded_rnti <= 0x000A || decoded_rnti > 0xFFF3)) {
					prob = 0;
				}

				/* checks whether the RNTI found is coherent with a scheduling in that location */
				if (!validate_rnti_in_search_space(agl, num_regs / 9,
						llr_buffer_record.get_subframe(), decoded_rnti, candidate * agl)) {
					prob = 0;
				}

				if (prob > best_prob) {
					best_prob = prob;
					best_format_idx = dci_format;
					best_decoded_rnti = decoded_rnti;
					best_decoded_dci_bits = decoded_dci_bits;

					if (prob == 1.0f) {  //we cannot get better than this
						break;
					}
				}
			}
			if (best_prob >= decode_success_prob_threshold) {
				//mark CCEs as DCI detected
				for (unsigned int cce = 0; cce < agl; cce++) {
					cce_dci_detected[candidate * agl + cce] = true;
				}

				srslte_dci_format_t best_format = active_dci_formats[best_format_idx];
				if (best_format == SRSLTE_DCI_FORMAT0) {
					if (best_decoded_dci_bits & 1) {
						best_format = SRSLTE_DCI_FORMAT1A;
					}
				}

				unsigned int harq_pid = (10 * llr_buffer_record.get_sfn()
						+ llr_buffer_record.get_subframe()) % 8;
				DciResult* dci_result = new DciResult(best_format,
						best_decoded_dci_bits, dci_format_lengths[best_format_idx],
						best_decoded_rnti, pdcch_add_cell_info_record->get_bandwidth_rb(),
						pdcch_add_cell_info_record->get_num_tx_ports(), harq_pid);
				dci_result->set_agl(agl);
				dci_result->set_start_cce(candidate * agl);
				dci_result->set_decoding_success_prob(best_prob);
				detected_dcis.push_back(dci_result);
			}
		}
	}

	/* verify that a DCI was decoded for all CCEs with detected "energy" */
	if (reg_energy_threshold > 0) {
		for (unsigned int cce = 0; cce < num_regs / 9; cce++) {
			if (!cce_dci_detected[cce]
					&& (active_cce[cce] >= DEFAULT_MIN_REGS_W_ENEGRY_CCE_ACTIVE)) {
				cout << "##########################" << endl;
				cout << "warning: CCE " << cce << " has high energy ("
						<< (int) active_cce[cce]
						<< " REGs over threshold), but no DCI detected!" << endl;
			}
			if (!active_cce[cce] && cce_dci_detected[cce]) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
				cout << "warning: CCE " << cce << " has small energy, but DCI detected!"
						<< endl;
			}
		}
	}

	/* callbacks */
	bool keep_dcis = false;
	if (callbacks.size() > 0) {
		for (unsigned int i = 0; i < callbacks.size(); i++) {
			keep_dcis |= callbacks[i](llr_buffer_record, detected_dcis, callback_args[i]);
		}
	}
	if (!keep_dcis) {
		for (list<DciResult*>::iterator it = detected_dcis.begin();
				it != detected_dcis.end(); it++) {
			delete *it;
		}
	}
}

bool PdcchDecoder::decode_record(PdcchLlrBufferRecord& llr_buffer_record) {
	if (pdcch_add_cell_info_record == 0) {
		return false;
	}
	/* pre-calculate values only depending on cell: */
	if (last_phy_cell_id != llr_buffer_record.get_phy_cell_id()) {
		pre_calculate_values(llr_buffer_record.get_phy_cell_id(),
				pdcch_add_cell_info_record->get_bandwidth_rb(),
				pdcch_add_cell_info_record->get_num_tx_ports());
	}

	/* read CCEs in right order,  */
	unsigned int used_cfi = llr_buffer_record.get_cfi();
	unsigned int num_regs = pdcch_add_cell_info_record->get_num_cce(used_cfi) * 9;

	int8_t cce_buf_raw[num_regs * 8];

	for (unsigned int j = 0; j < num_regs; j++) {
		reg_idx r = cce_reg_to_reg_pos_mapping[used_cfi - 1][j];
		unsigned int reg_nr = r.reg_nr;
		if (r.sym_idx >= 1) {
			reg_nr += pdcch_add_cell_info_record->get_bandwidth_rb()
					* pdcch_add_cell_info_record->get_regs_per_rb(0);
		}
		if (r.sym_idx == 2) {
			reg_nr += pdcch_add_cell_info_record->get_bandwidth_rb()
					* pdcch_add_cell_info_record->get_regs_per_rb(1);
		}
		llr_buffer_record.get_reg_values(reg_nr, cce_buf_raw + j * 8);
	}

	/* de-scrambling (and inversion of LLR values) */
	int16_t cce_buf[num_regs * 8];
	for (unsigned int i = 0; i < num_regs * 8; i++) {
		if (scramble_seq[llr_buffer_record.get_subframe()][i] == 0) {
			cce_buf[i] = -1 * cce_buf_raw[i];
		} else {
			cce_buf[i] = 1 * cce_buf_raw[i];
		}
	}

	/* blind decoding */
	blind_decode(cce_buf, num_regs, llr_buffer_record);

	return true;
}

bool pdcch_decoder_process_data_record(PdcchLlrBufferRecord* llr_buffer_record,
		void* arg) {
	PdcchDecoder* pdcch_decoder = (PdcchDecoder*) arg;
	pdcch_decoder->decode_record(*llr_buffer_record);

	return false;
}

bool pdcch_decoder_process_add_cell_info_record(
		PdcchAddCellInfoRecord* pdcch_add_cell_info_record, void* arg) {
	PdcchDecoder* pdcch_decoder = (PdcchDecoder*) arg;
	return pdcch_decoder->new_pdcch_add_cell_info_record(
			pdcch_add_cell_info_record);
}

void PdcchDecoder::connect_to_record_reader(
		PdcchDumpRecordReader& pdcch_dump_record_reader) {
	pdcch_dump_record_reader.register_callback(PDCCH_LLR_BUFFER_RECORD,
			(record_callback_t) &pdcch_decoder_process_data_record, this);
	pdcch_dump_record_reader.register_callback(PDCCH_ADD_CELL_INFO_RECORD,
			(record_callback_t) &pdcch_decoder_process_add_cell_info_record, this);
}
