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

	void set_exclude_cqi_request_only(bool exclude_cqi_request_only);
	void set_exclude_retransmission_mcs(bool exclude_retransmission_mcs);
private:
	void update_fields();

	bool exclude_cqi_request_only;
	bool exclude_retransmission_mcs;
};

#endif /* ULMCSANALYZER_H_ */
