 /**
 * @file dsat_wrapper.h
 * @brief Definitions of existing baseband firmware AT commands functions
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __DSAT_WRAPPER_H
#define __DSAT_WRAPPER_H

ADDRESS(0x08DF6FA0) void dsat_process_cmd_line(unsigned int at_state, unsigned char* cmd_line_ptr);

ADDRESS(0x08DF5824) void dsatrsp_send_response(void* rsp_ptr, unsigned char rearm_autodetect);
ADDRESS(0x08E09508) void dsat_reg_rsp_router(void* func_ptr);

#endif
