 /**
 * @file tasks_wrapper.h
 * @brief Definitions of existing baseband firmware functions related to tasks (rcinit)
 * 
 * Specific to Asus Padfone Infinity 2 (A86), firmware version "M3.12.15-A86_1032"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __TASKS_WRAPPER_H
#define __TASKS_WRAPPER_H

ADDRESS(0x979E110) void rcinit_internal_start_daltask(void* rcinit_p, void* entry);
ADDRESS(0x979E190) void rcinit_internal_start_posix(void* rcinit_p, void* entry);
ADDRESS(0x979E320) void rcinit_internal_start_qurttask(void* rcinit_p, void* entry);
ADDRESS(0x979E550) void rcinit_internal_start_rextask(void* rcinit_p, void* entry);

ADDRESS(0x9F46BB0) void qurt_thread_create(void* thread_id, void* attr, void* entrypoint, void *arg);

ADDRESS(0x9271A30) int pthread_create_2(void* tid, void* attr, void* start, void* arg, int unknown_param);

#endif 
