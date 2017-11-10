/**
 * @file clocks.h
 * @brief Definitions of existing baseband firmware functions related to clocks and power
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __CLOCKS_H
#define __CLOCKS_H

ADDRESS(0x09599840) void HAL_clk_GetPowerDomainName(void* pPowerDomainHandle, const char **szPowerDomainName);
ADDRESS(0x095996C0) void HAL_clk_GetClockName(void* pClockHandle, const char **szClockName);
ADDRESS(0x095992B0) void HAL_clk_EnableClock(void* pClockHandle);
ADDRESS(0x09599380) void HAL_clk_DisableClock(void* pClockHandle);
ADDRESS(0x09599390) unsigned int HAL_clk_IsClockOn(void* pClockHandle);

ADDRESS(0x095936F0) unsigned int Clock_DriverInit(void* pDrvCtxt);

ADDRESS(0x09593FF0) unsigned int Clock_GetClockId(void* pDrvCtxt, const char* szClock, unsigned int* pnId);
ADDRESS(0x095942F0) unsigned int Clock_EnableClock(void* pDrvCtxt, unsigned int nClock);
ADDRESS(0x09594410) unsigned int Clock_DisableClock(void* pDrvCtxt, unsigned int nClock);

ADDRESS(0x09595230) unsigned int Clock_GetSourceId(void* pDrvCtxt, const char* szClock, unsigned int* pnId);
ADDRESS(0x09595240) unsigned int Clock_EnableSource(void* pDrvCtxt, unsigned int nSource);
ADDRESS(0x095952A0) unsigned int Clock_DisableSource(void* pDrvCtxt, unsigned int nSource);

ADDRESS(0x09594E60) unsigned int Clock_GetPowerDomainId(void* pDrvCtxt, const char* szClock, unsigned int* pnPowerDomainId);

#endif
