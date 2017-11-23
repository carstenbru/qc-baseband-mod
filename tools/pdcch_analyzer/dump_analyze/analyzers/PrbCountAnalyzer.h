/*
 PrbCountAnalyzer.h
 
 Simple analyzer counting the number of allocated PRBs (total, paging, UE)

 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PRBCOUNTANALYZER_H_
#define PRBCOUNTANALYZER_H_

#include "SubframeAnalyzer.h"

class PrbCountAnalyzer: public SubframeAnalyzer {
public:
	PrbCountAnalyzer();
	virtual ~PrbCountAnalyzer();

	virtual bool analyze_subframe(PdcchDciRecord* dci_record, PdcchDumpRecordReader* pdcch_dump_record_reader);
};

#endif /* PRBCOUNTANALYZER_H_ */
