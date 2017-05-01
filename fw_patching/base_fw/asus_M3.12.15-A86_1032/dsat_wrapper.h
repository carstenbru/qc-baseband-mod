 /**
 * @file dsat_wrapper.h
 * @brief Definitions of existing baseband firmware AT commands functions
 * 
 * Specific to Asus Padfone Infinity 2 (A86), firmware version "M3.12.15-A86_1032"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __DSAT_WRAPPER_H
#define __DSAT_WRAPPER_H

ADDRESS(0x8B00FFC) void dsat_process_cmd_line(unsigned int at_state, unsigned char* cmd_line_ptr);

ADDRESS(0x8B10EA0) void dsatrsp_send_response(void* rsp_ptr, unsigned char rearm_autodetect);
ADDRESS(0x8B10690) void dsat_reg_rsp_router(void* func_ptr);

#endif
