/**
 * @file MPSS.DI.3.0-0f11eec_wrapper.h
 * @brief Definitions of existing baseband firmware functions
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __FW_WRAPPER_H
#define __FW_WRAPPER_H

/* define address attribute short definition */
#define ADDRESS(x) __attribute__ ((address (x)))

#include "qmi_functions_wrapper.h"
#include "lte_mac_wrapper.h"
#include "lte_sec_wrapper.h"
#include "dsat_wrapper.h"
#include "channel_est_wrapper.h"
#include "tasks_wrapper.h"
#include "clocks.h"
#include "pdcch.h"
#include "hw_regs.h"
//#include "experimental.h"

/* ########################################### */

/* define patch code target address (as void pointer) */

ADDRESS(0x09658900) void* __patch_addr_text_base__;
/* you can also define the start address for data sections,
   if left undefined they will be placed directly after the code */
ADDRESS(0x0CB98000) void* __patch_addr_data_base__;
/* you can also define the start address for data sections accessed by code with restricted rights (e.g. by memcpy, memset),
   if left undefined they will be placed together with the other data */
ADDRESS(0x0CB7F000) void* __patch_addr_data_restricted_base__;


/* ########################################### */
       
/* firmware identification strings*/

ADDRESS(0x097836E8) extern char* fw_version_string;
ADDRESS(0x097836A0) extern char* fw_time_string;
ADDRESS(0x097836D8) extern char* fw_time_string2;
ADDRESS(0x09783690) extern char* fw_date_string;
ADDRESS(0x097836C8) extern char* fw_date_string2;

/* hexagon helper functions */

ADDRESS(0x08010000) void __save_r16_through_r27(void);
ADDRESS(0x08010020) void __save_r16_through_r25(void);
ADDRESS(0x08010008) void __save_r16_through_r23(void);
ADDRESS(0x08010028) void __save_r16_through_r21(void);
ADDRESS(0x08010010) void __save_r16_through_r19(void);

ADDRESS(0x08010080) void __restore_r16_through_r27_and_deallocframe(void);
ADDRESS(0x08010084) void __restore_r16_through_r25_and_deallocframe(void);
ADDRESS(0x08010070) void __restore_r16_through_r23_and_deallocframe(void);
ADDRESS(0x08010090) void __restore_r16_through_r21_and_deallocframe(void);
ADDRESS(0x08010078) void __restore_r16_through_r19_and_deallocframe(void);
ADDRESS(0x08010098) void __restore_r16_through_r17_and_deallocframe(void);

ADDRESS(0x08010048) void __restore_r16_through_r27_and_deallocframe_before_tailcall(void);
ADDRESS(0x08010050) void __restore_r16_through_r25_and_deallocframe_before_tailcall(void);
ADDRESS(0x08010038) void __restore_r16_through_r23_and_deallocframe_before_tailcall(void);
ADDRESS(0x08010058) void __restore_r16_through_r21_and_deallocframe_before_tailcall(void);
ADDRESS(0x08010040) void __restore_r16_through_r19_and_deallocframe_before_tailcall(void);
ADDRESS(0x08010060) void __restore_r16_through_r17_and_deallocframe_before_tailcall(void);

/* standard C functions */

ADDRESS(0x084A21D4) void* malloc(unsigned int size);
ADDRESS(0x084A3B30) void free(void* ptr);

ADDRESS(0x0955B460) int snprintf(char* str, unsigned int size, const char* format, ...);
ADDRESS(0x08010100) void* memcpy(void* destination, const void* source, unsigned int num);
ADDRESS(0x084A6FB0) void* memcpy_s(void* destination, unsigned int dest_size, const void* source, unsigned int num);

ADDRESS(0x080103F0) void* memset(void* ptr, int value, unsigned int num);

ADDRESS(0x09559740) int strcmp(char* str1, char* str2);

#endif /* __FW_WRAPPER_H */ 
