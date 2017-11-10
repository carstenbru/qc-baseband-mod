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

#include "../records/PdcchDataRecord.h"
#include "../records/PdcchAddCellInfoRecord.h"

#include "../PdcchDumpRecordReader.h"
#include "DciResult.h"

extern "C" {
#include "srsLTE/dci.h"
}

typedef struct {
	uint16_t sym_idx;
	uint16_t reg_nr;
} reg_idx;

/**
 * callback function type
 *
 * @param arg argument defined by user
 * @return true if the passed DciResults are still needed, i.e. should not be deleted
 */
typedef bool (*decoder_callback_t)(PdcchDataRecord& data_record,
		std::list<DciResult*> decoded_dcis, void* arg);

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
	bool decode_record(PdcchDataRecord& data_record);

	bool new_pdcch_add_cell_info_record(
			PdcchAddCellInfoRecord* pdcch_add_cell_info_record);

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
private:
	/**
	 * pre-calculates values only depending on cell for fast decoding
	 */
	void pre_calculate_values(uint16_t phy_cell_id, unsigned int prbs,
			unsigned int tx_ports);
	void blind_decode(int16_t* cce_buf, unsigned int num_regs,
			PdcchDataRecord& data_record);
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

	std::vector<decoder_callback_t> callbacks;
	std::vector<void*> callback_args;

	float decode_success_prob_threshold;

	PdcchAddCellInfoRecord* pdcch_add_cell_info_record;

	// pre-calculated values (only depending on cell)
	int16_t last_phy_cell_id;
	reg_idx* cce_reg_to_reg_pos_mapping[3];
	uint32_t* scramble_seq[10];
	unsigned int num_dci_formats;
	unsigned int dci_format_lengths[SRSLTE_DCI_NOF_FORMATS];

	unsigned int reg_energy_threshold;
};

#endif /* PDCCHDECODER_H_ */
