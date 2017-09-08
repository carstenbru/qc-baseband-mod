 /**
 * @file tasks_wrapper.h
 * @brief Definitions of existing baseband firmware functions related to tasks (rcinit)
 * 
 * Specific to Xiaomi Mi4 LTE, firmware version "MPSS.DI.3.0-0f11eec"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __TASKS_WRAPPER_H
#define __TASKS_WRAPPER_H

ADDRESS(0x08742340) void rcinit_internal_start_daltask(void* rcinit_p, void* entry);
ADDRESS(0x087425B0) void rcinit_internal_start_posix(void* rcinit_p, void* entry);
ADDRESS(0x087423B0) void rcinit_internal_start_qurttask(void* rcinit_p, void* entry);
ADDRESS(0x087421B0) void rcinit_internal_start_rextask(void* rcinit_p, void* entry);

ADDRESS(0x0953FE14) void qurt_thread_create(void* thread_id, void* attr, void* entrypoint, void *arg);

ADDRESS(0x095546D0) int pthread_create_2(void* tid, void* attr, void* start, void* arg, int unknown_param);

#endif 
