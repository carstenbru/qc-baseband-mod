/*
 * PdcchDecoder.h
 *
 * large parts of the actual decoder are adapted from the srsLTE project
 *
 * Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHDECODER_H_
#define PDCCHDECODER_H_

#include <vector>
#include <list>

#include "../records/PdcchDciRecord.h"
#include "../records/PdcchAddCellInfoRecord.h"

#include "DciResult.h"

extern "C" {
#include "srsLTE/dci.h"
#include "srsLTE/viterbi.h"
#include "srsLTE/convcoder.h"
}

typedef struct {
	uint16_t sym_idx;
	uint16_t reg_nr;
} reg_idx;

class PdcchDumpRecordReader;

#define DEFAULT_INACTIVITY_TIME_MS 20000
#define DEFAULT_INIT_PERIOD_MS 30000
#define DEFAULT_RACH_TIMEOUT_MS 200

/**
 * callback function type
 *
 * @param arg argument defined by user
 * @return true if the passed PdcchDciRecord is still needed, i.e. should not be deleted
 */
typedef bool (*decoder_callback_t)(PdcchDciRecord* pdcch_dci_record, void* arg);

class PdcchDecoder {
public:
	PdcchDecoder();
	virtual ~PdcchDecoder();

	/**
	 * identifies and decodes all DCIs in a data record
	 *
	 * The results are available by the result callback, so such a callback should be registered before
	 *
	 * @return true if decoding succeeded, otherwise false (e.g. no previous PdcchAddCellInfoRecord)
	 */
	bool decode_record(PdcchLlrBufferRecord& llr_buffer_record);

	/**
	 * register a callback for decoding results
	 *
	 * @param callback callback function
	 * @param arg user argument that will be passed to the callback function
	 */
	void register_callback(decoder_callback_t callback, void* arg);

	/**
	 * sets the threshold for a decoding to be considered successful
	 *
	 * Note that in any case the best probability over all formats for a location and an AGL
	 * is calculated and then a decision is made based on this threshold. If the result is rejected,
	 * the next smaller AGL is tried (as a wrong match in a higher AGL is less likely and thus
	 * a result here is considered better)
	 */
	void set_decode_success_prob_threshold(float decode_success_prob_threshold) {
		this->decode_success_prob_threshold = decode_success_prob_threshold;
	}

	/**
	 * sets the threshold for a decoding with a known RNTI value (P/SI/RA-RNTI or active C-RNTI) to be accepted immediately, i.e. its probability being set to 1.0
	 */
	void set_decode_success_prob_known_rnti_threshold(
			float decode_success_prob_known_rnti_threshold) {
		this->decode_success_prob_known_rnti_threshold =
				decode_success_prob_known_rnti_threshold;
	}

	/**
	 * sets the threshold for REGs "energy" to be considered as containing data
	 *
	 * The absolute sum of all LLR values in an REG has to be bigger than this threshold,
	 * set it to 0 to disable the feature
	 */
	void set_reg_energy_threshold(unsigned int reg_energy_threshold) {
		this->reg_energy_threshold = reg_energy_threshold;
	}

	/**
	 * connect to a PdcchDumpRecordReader
	 *
	 * The decoder registers callbacks at the record reader and will automatically decode all read dumps.
	 * In order to get the decoding, register a callback to the decoder with register_callback.
	 */
	void connect_to_record_reader(
			PdcchDumpRecordReader& pdcch_dump_record_reader);

	/**
	 * sets the time (in ms) after which a RNTI is considered as not active anymore
	 */
	void set_inactivity_time_ms(unsigned int inactivity_time_ms) {
		this->inactivity_time_ms = inactivity_time_ms;
	}
	/**
	 * sets the time (in ms) which is considered as init period, i.e. in which the list of active RNTIs is initialized
	 */
	void set_init_period_ms(unsigned int init_period_ms) {
		this->init_period_ms = init_period_ms;
	}

	/**
	 * prints various statistic values, can be called after decoding a set of records
	 */
	void print_stats();

	/**
	 * set to true to output warning messages to stdout
	 */
	void set_report_warnings(bool report_warnings) {
		this->report_warnings = report_warnings;
	}
private:
	/**
	 * pre-calculates values only depending on cell for fast decoding
	 */
	void pre_calculate_values(uint16_t phy_cell_id, unsigned int prbs,
			unsigned int tx_ports);
	void blind_decode(int16_t* cce_buf, unsigned int num_regs,
			PdcchLlrBufferRecord& llr_buffer_record, uint64_t current_time);
	/**
	 * validates if a CCE is part of the search space of a given RNTI
	 */
	bool validate_rnti_in_search_space(unsigned int agl, unsigned int num_cce,
			unsigned int subframe, uint16_t rnti, unsigned int start_cce);
	/**
	 * tries to decode a DCI from channel bits, adapted from srsLTE
	 *
	 * @param ch_data pointer to first value, each byte contains one LLR value
	 * @param agl aggregation level
	 * @param dci_bits number of bits in DCI format to try
	 * @param decoded_rnti resulting RNTI value
	 * @param decoded_dci_bits resulting DCI bits
	 */
	float try_decode_dci(int16_t* ch_data, unsigned int agl,
			unsigned int dci_bits, uint16_t* decoded_rnti,
			uint64_t* decoded_dci_bits);
	/**
	 * checks if RNTIs got inactive at the current timepoint and thus should be removed from the list of active RNTIs
	 */
	void check_rntis_inactive(uint64_t current_time);
	/**
	 * check if some of the observed RACH procedures are longer ago than the defined threshold
	 */
	void check_rach_timeout(uint64_t current_time);

	std::vector<decoder_callback_t> callbacks;
	std::vector<void*> callback_args;

	float decode_success_prob_threshold;
	float decode_success_prob_known_rnti_threshold;

	// pre-calculated values (only depending on cell)
	int16_t last_phy_cell_id;
	reg_idx* cce_reg_to_reg_pos_mapping[3];
	uint32_t* scramble_seq[10];
	unsigned int num_dci_formats;
	unsigned int dci_format_lengths[SRSLTE_DCI_NOF_FORMATS];

	unsigned int reg_energy_threshold;

	srslte_viterbi_t decoder;
	srslte_convcoder_t encoder;

	PdcchDumpRecordReader* pdcch_dump_record_reader;

	unsigned int analyzed_subframes;
	unsigned int detected_decoded_dcis;
	unsigned int cces_w_energy_no_dci;
	unsigned int cces_w_dci_no_energy;

	unsigned int inactivity_time_ms;
	uint64_t rnti_last_seen[65536];
	std::list<unsigned int> active_rntis;

	unsigned int rach_timeout_ms;
	std::list<uint64_t> observed_rach;
	unsigned int rnti_active_after_rach;
	unsigned int rnti_active_wo_rach;
	unsigned int rach_wo_new_rnti;

	unsigned int init_period_ms;

	bool report_warnings;
	uint64_t last_time;
};

#endif /* PDCCHDECODER_H_ */
