/*
 DataRateAnalyzer.h
 
 Simple analyzer computing the allocated data rate (total, paging, UE)

 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef DATARATEANALYZER_H_
#define DATARATEANALYZER_H_

#include "SubframeAnalyzer.h"

class DataRateAnalyzer: public SubframeAnalyzer {
public:
	DataRateAnalyzer();
	virtual ~DataRateAnalyzer();

	virtual bool analyze_subframe(PdcchDciRecord* dci_record, PdcchDumpRecordReader* pdcch_dump_record_reader);
};

#endif /* DATARATEANALYZER_H_ */
