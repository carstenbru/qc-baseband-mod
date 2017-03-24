/**
 * @file task_creation.c
 * @brief task creation hooks main source file
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER

#define TASK_LIST_SIZE 8192*4

char task_list[TASK_LIST_SIZE];
int task_list_write_pos = 0;

void write_task_to_list(void* rcinit_p, char* type) {
    int max_size = TASK_LIST_SIZE - task_list_write_pos;
    if (max_size < 0) {
        max_size = 0;
    }
    int priority = *((unsigned short*)(rcinit_p + 0xC));
    char* task_name = *((char**)(rcinit_p + 0));
    task_list_write_pos += snprintf(task_list + task_list_write_pos, max_size, "rcinit starting task: %s, %d\n", task_name, priority);
}

__attribute__ ((overwrite ("rcinit_internal_start_daltask")))
void rcinit_internal_start_daltask_hook(void* rcinit_p, void* entry) {
    write_task_to_list(rcinit_p, "dal");
    rcinit_internal_start_daltask_fw_org(rcinit_p, entry);
}

__attribute__ ((overwrite ("rcinit_internal_start_posix")))
void rcinit_internal_start_posix_hook(void* rcinit_p, void* entry) {
    write_task_to_list(rcinit_p, "posix");
    rcinit_internal_start_posix_fw_org(rcinit_p, entry);
}

__attribute__ ((overwrite ("rcinit_internal_start_qurttask")))
void rcinit_internal_start_qurttask_hook(void* rcinit_p, void* entry) {
    write_task_to_list(rcinit_p, "qurt");
    rcinit_internal_start_qurttask_fw_org(rcinit_p, entry);
}

__attribute__((overwrite ("rcinit_internal_start_rextask")))
void rcinit_internal_start_rextask_hook(void* rcinit_p, void* entry) {
    write_task_to_list(rcinit_p, "rex");
    rcinit_internal_start_rextask_fw_org(rcinit_p, entry);
}

__attribute__((overwrite ("qurt_thread_create")))
void qurt_thread_create_hook(void* thread_id, void* attr, void* entrypoint, void *arg) {
    int max_size = TASK_LIST_SIZE - task_list_write_pos;
    if (max_size < 0) {
        max_size = 0;
    }
    char* task_name = (char*)(attr + 0);
    int priority = *((unsigned short*)(attr + 0x12));
    task_list_write_pos += snprintf(task_list+task_list_write_pos, max_size, "qurt_thread_create: %s, %d\n", task_name, priority);
    
    qurt_thread_create_fw_org(thread_id, attr, entrypoint, arg);
}

// __attribute__((overwrite ("pthread_create_2")))
// int pthread_create_hook(void* tid,void* attr, void* start, void* arg, int unknown_param) {
//     int max_size = TASK_LIST_SIZE - task_list_write_pos;
//     if (max_size < 0) {
//         max_size = 0;
//     }
//     char* task_name = (char*)(attr + 0x18);
//     task_list_write_pos += snprintf(task_list+task_list_write_pos, max_size, "pthread_create: %s\n", task_name);
//     
//     return pthread_create_2_fw_org(tid, attr, start, arg, unknown_param);
// }
