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

#define PTHREAD_NAME_LEN 16

typedef struct pthread_attr_t
{
    void*        stackaddr;
    int          internal_stack; /* this flag==1 means the stack needs to be freed by posix */
    signed int   stacksize;
    int          priority;
    unsigned int timetest_id;    
    unsigned int cpumask;
    char         name[PTHREAD_NAME_LEN];
    /* This flag indicates whether pthread lib should create thread contexts for other OSALs */
    /* This is used internally by POSIX and not available for general usage */
    int          ext_context;
} pthread_attr_t;

ADDRESS(0x08742340) void rcinit_internal_start_daltask(void* rcinit_p, void* entry);
ADDRESS(0x087425B0) void rcinit_internal_start_posix(void* rcinit_p, void* entry);
ADDRESS(0x087423B0) void rcinit_internal_start_qurttask(void* rcinit_p, void* entry);
ADDRESS(0x087421B0) void rcinit_internal_start_rextask(void* rcinit_p, void* entry);

ADDRESS(0x0953FE14) void qurt_thread_create(void* thread_id, void* attr, void* entrypoint, void *arg);

ADDRESS(0x095546D0) int pthread_create_2(void* tid, void* attr, void* start, void* arg, int unknown_param);

//TODO add other known pthread functions
//TODO maybe move pthread functions into own header, since they are a separate library
//always call "pthread_create" WITH attributes (even though 0 usually allowed), otherwise firmware might crash randomly later
ADDRESS(0x09554AA8) int pthread_create(unsigned int* tid, pthread_attr_t* attr, void* start, void* arg);
ADDRESS(0x09554AF0) void pthread_exit(void* value_ptr);

ADDRESS(0x09554CB0) int pthread_attr_init(pthread_attr_t* attr);
ADDRESS(0x09554CFC) int pthread_attr_destroy(pthread_attr_t* attr);
ADDRESS(0x09554D68) int pthread_attr_setthreadname(pthread_attr_t* attr, const char* name);
ADDRESS(0x09554D3C) int pthread_attr_setstacksize(pthread_attr_t* attr, signed int stacksize);

ADDRESS(0x09555274) unsigned int pthread_self();
ADDRESS(0x09554BF0) int pthread_setschedprio(unsigned int thread, int prio);

ADDRESS(0x09555EB0) int sem_init(unsigned int* sem, int pshared, unsigned int value);
ADDRESS(0x09555F24) int sem_wait(unsigned int* sem);
ADDRESS(0x09555F80) int sem_post(unsigned int* sem);

#endif 
