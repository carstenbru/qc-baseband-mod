/*
 UlMcsAnalyzer.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef ULMCSANALYZER_H_
#define ULMCSANALYZER_H_

#include "SubframeAnalyzer.h"

class UlMcsAnalyzer: public SubframeAnalyzer {
public:
	UlMcsAnalyzer();
	virtual ~UlMcsAnalyzer();

	virtual bool analyze_subframe(PdcchDciRecord* dci_record,
			PdcchDumpRecordReader* pdcch_dump_record_reader);

	/**
	 * if set to true, DCIs requesting only a CQI report (and no user data) are excluded from the distribution
	 */
	void set_exclude_cqi_request_only(bool exclude_cqi_request_only);
	/*
	 * if set to true, DCIs for retransmissions are excluded from the distribution
	 */
	void set_exclude_retransmission_mcs(bool exclude_retransmission_mcs);
	/**
	 * if set to true, DCIs with the RNTI value currently assigned to the dumping UE are excluded from the distribution
	 */
	void set_exclude_own_rnti(bool exclude_own_rnti) {
		this->exclude_own_rnti = exclude_own_rnti;
	}

	virtual bool set_parameter(std::string name, std::vector<std::string>& values);
private:
	void update_fields();

	bool exclude_cqi_request_only;
	bool exclude_retransmission_mcs;
	bool exclude_own_rnti;
};

#endif /* ULMCSANALYZER_H_ */
