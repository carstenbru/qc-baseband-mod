/*
 PdcchAddCellInfoRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchAddCellInfoRecord.h"

using namespace std;

PdcchAddCellInfoRecord::PdcchAddCellInfoRecord(unsigned int record_version,
		char* data, unsigned int length) :
		PdcchDumpRecord(record_version, data, length) {

}

PdcchAddCellInfoRecord::~PdcchAddCellInfoRecord() {
}

string PdcchAddCellInfoRecord::to_string() {
	char buf[256];
	snprintf(buf, sizeof(buf),
			"PdcchAddCellInfoRecord: phy cell ID: %d\tbandwidth: %.2fMHz\tnumber of CCEs for CFI1: %d\tnum_tx_antennas: %d\tnum_rx_antennas: %d\tRNTI of UE: %d",
			get_phy_cell_id(), get_bandwidth_MHz(), get_num_cce(1),
			get_num_tx_antennas(), get_num_rx_antennas(), get_rnti(C_RNTI));

	return string(buf);
}

char* PdcchAddCellInfoRecord::get_reg_map(unsigned int symb) {
	unsigned int map_sel;
	if (symb == 0) {
		map_sel = ((*((uint32_t*) (data + 216)) >> 2) & 0x3);
		return (data + 12 + map_sel * 28);
	} else if (symb == 1) {
		map_sel = ((*((uint32_t*) (data + 216)) >> 4) & 0x1);
		return (data + 96 + map_sel * 40);
	} else {
		return (data + 176);
	}
}
