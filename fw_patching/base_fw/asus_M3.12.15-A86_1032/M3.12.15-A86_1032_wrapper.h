/**
 * @file M3.12.15-A86_1032_wrapper.h
 * @brief Definitions of existing baseband firmware functions
 * 
 * Specific to Asus Padfone Infinity 2 (A86), firmware version "M3.12.15-A86_1032"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __FW_WRAPPER_H
#define __FW_WRAPPER_H

/* define address attribute short definition */
#define ADDRESS(x) __attribute__ ((address (x)))

#include "qmi_functions_wrapper.h"
#include "lte_mac_wrapper.h"

/* ########################################### */

/* define patch code target address (as void pointer) */

ADDRESS(0x09F94000) void* __patch_addr_text_base__;
/* you can also define the start address for data sections,
   if left undefined they will be placed directly after the code */
ADDRESS(0x0CD0F000) void* __patch_addr_data_base__;

/* ########################################### */
       
/* firmware identification strings*/

ADDRESS(0x0A0FB088) char* fw_version_string;
ADDRESS(0x0A0FB0A0) char* fw_time_string;
ADDRESS(0x0A0FB0D0) char* fw_time_string2;
ADDRESS(0x0A0FB0B0) char* fw_date_string;
ADDRESS(0x0A0FB0E0) char* fw_date_string2;

/* hexagon helper functions */

ADDRESS(0x09130000) void __save_r16_through_r27(void);
ADDRESS(0x09130020) void __save_r16_through_r25(void);
ADDRESS(0x09130008) void __save_r16_through_r23(void);
ADDRESS(0x09130028) void __save_r16_through_r21(void);
ADDRESS(0x09130010) void __save_r16_through_r19(void);

ADDRESS(0x09130080) void __restore_r16_through_r27_and_deallocframe(void);
ADDRESS(0x09130084) void __restore_r16_through_r25_and_deallocframe(void);
ADDRESS(0x09130070) void __restore_r16_through_r23_and_deallocframe(void);
ADDRESS(0x09130090) void __restore_r16_through_r21_and_deallocframe(void);
ADDRESS(0x09130078) void __restore_r16_through_r19_and_deallocframe(void);
ADDRESS(0x09130098) void __restore_r16_through_r17_and_deallocframe(void);

ADDRESS(0x09130048) void __restore_r16_through_r27_and_deallocframe_before_tailcall(void);
ADDRESS(0x09130050) void __restore_r16_through_r25_and_deallocframe_before_tailcall(void);
ADDRESS(0x09130038) void __restore_r16_through_r23_and_deallocframe_before_tailcall(void);
ADDRESS(0x09130058) void __restore_r16_through_r21_and_deallocframe_before_tailcall(void);
ADDRESS(0x09130040) void __restore_r16_through_r19_and_deallocframe_before_tailcall(void);
ADDRESS(0x09130060) void __restore_r16_through_r17_and_deallocframe_before_tailcall(void);

/* standard C functions */

ADDRESS(0x09F88260) void* malloc(unsigned int size);
ADDRESS(0x09F87F40) void free(void* ptr);

ADDRESS(0x092B9AB0) int snprintf(char* str, unsigned int size, const char* format, ...);
ADDRESS(0x09130360) void* memcpy(void* destination, const void* source, unsigned int num);
ADDRESS(0x09130650) void* memset(void* ptr, int value, unsigned int num);

#endif /* __FW_WRAPPER_H */ 
