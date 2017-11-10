/*
 PdcchMainCellInfoRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchMainCellInfoRecord.h"

using namespace std;

PdcchMainCellInfoRecord::PdcchMainCellInfoRecord(unsigned int record_version,
		char* data, unsigned int length) :
		PdcchDumpRecord(record_version, data, length) {

}

PdcchMainCellInfoRecord::~PdcchMainCellInfoRecord() {
}

bool PdcchMainCellInfoRecord::is_same_cell(PdcchMainCellInfoRecord other_cell) {
	return ((get_phy_cell_id() == other_cell.get_phy_cell_id())
			&& (get_mcc() == other_cell.get_mcc())
			&& (get_mnc() == other_cell.get_mnc())
			&& (get_tac() == other_cell.get_tac())
			&& (get_cid() == other_cell.get_cid()));
}

string PdcchMainCellInfoRecord::to_string() {
	char buf[256];
	snprintf(buf, sizeof(buf),
			"PdcchMainCellInfoRecord: MCC: %03d\tMNC: %03d\tTAC: %d\tCID: %d\tphy cell ID: %d", get_mcc(),
			get_mnc(), get_tac(), get_cid(), get_phy_cell_id());

	return string(buf);
}
