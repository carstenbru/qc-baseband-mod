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

	/**
	 * if set to true, special RNTI values (paging RNTI, system information RNTI, ...) are excluded from the distribution
	 *
	 * only DCIs with RNTIs in [0x0001, 0xFFF3] are evaluated then
	 */
	void set_exclude_special_rntis(bool exclude_special_rntis);

	/**
	 * if set to true, DCIs with the RNTI value currently assigned to the dumping UE are excluded from the distribution
	 */
	void set_exclude_own_rnti(bool exclude_own_rnti);
private:
	bool exclude_special_rntis;
	bool exclude_own_rnti;
};

#endif /* DLMCSANALYZER_H_ */
