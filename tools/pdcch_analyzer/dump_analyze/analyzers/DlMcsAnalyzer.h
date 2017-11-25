/*
 DlMcsAnalyzer.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef DLMCSANALYZER_H_
#define DLMCSANALYZER_H_

#include "SubframeAnalyzer.h"

class DlMcsAnalyzer: public SubframeAnalyzer {
public:
	DlMcsAnalyzer();
	virtual ~DlMcsAnalyzer();

	virtual bool analyze_subframe(PdcchDciRecord* dci_record, PdcchDumpRecordReader* pdcch_dump_record_reader);
};

#endif /* DLMCSANALYZER_H_ */
