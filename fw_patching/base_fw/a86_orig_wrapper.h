/*
 * fw_wrapper.h
 * 
 * Definitions of original firmware functions
 * 
 * @author Carsten Bruns
 */

#ifndef __FW_WRAPPER_H
#define __FW_WRAPPER_H

/* define address attribute short definition */
#define ADDRESS(x) __attribute__ ((address (x)))

/* ########################################### */

ADDRESS(0xA0FB088) char* fw_version_string;
ADDRESS(0xA0FB0A0) char* fw_time_string;
ADDRESS(0xA0FB0D0) char* fw_time_string2;
ADDRESS(0xA0FB0B0) char* fw_date_string;
ADDRESS(0xA0FB0E0) char* fw_date_string2;

//TODO fill with real functions and addresses instead of test dummys

ADDRESS(0x08400100) void printf2(char*);
ADDRESS(0x08400000) void add(int a, int b);

#endif /* __FW_WRAPPER_H */ 
