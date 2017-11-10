/**
 * @file hw_regs.h
 * @brief Definitions of existing baseband hardware registers 
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __HW_REGS_H
#define __HW_REGS_H

ADDRESS(0xECB20004) extern volatile unsigned int MODEM_PWRUP;
ADDRESS(0xECB4A204) extern volatile unsigned int PDCCH_DEINT_CFG_WORD1;
ADDRESS(0xECB4A304) extern volatile unsigned int SYMBOL0_REG_MAP_0_WORD1;
ADDRESS(0xECB4A404) extern volatile unsigned int SYMBOL0_REG_MAP_1_WORD1;
ADDRESS(0xECB4A504) extern volatile unsigned int SYMBOL0_REG_MAP_2_WORD1;
ADDRESS(0xECB4A604) extern volatile unsigned int SYMBOL1_REG_MAP_0_WORD1;
ADDRESS(0xECB4A704) extern volatile unsigned int SYMBOL1_REG_MAP_1_WORD1;
ADDRESS(0xECB4A804) extern volatile unsigned int SYMBOL2_REG_MAP_WORD1;
ADDRESS(0xECB4AA00) extern volatile unsigned int PDCCH_REG_REORDER_WORD0;
ADDRESS(0xECB4AA04) extern volatile unsigned int PDCCH_REG_REORDER_WORD1;
ADDRESS(0xECB50110) extern volatile unsigned int TBVD_CCH_LTE_CFG_WORD4;

ADDRESS(0xECB38004) extern volatile unsigned int MEM_PL_BRDG_CH0_CFG0;
ADDRESS(0xECB38008) extern volatile unsigned int MEM_PL_BRDG_CH0_CFG1;
ADDRESS(0xECB3800C) extern volatile unsigned int MEM_PL_MEM_CH0_PAGE_NUM;
ADDRESS(0xECB42004) extern volatile unsigned int LTE_DBE_PCFICH_STATUS;

#endif
