#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

#ifdef _WIN32
#else
typedef pthread_mutex_t OSSyncMutex;
typedef pthread_cond_t OSSyncCondVar;
#endif

int os_mutex_init(OSSyncMutex *mutex);

void os_mutex_destroy(OSSyncMutex *mutex);

void os_mutex_lock(OSSyncMutex *mutex);

void os_mutex_unlock(OSSyncMutex *mutex);

int os_condvar_init(OSSyncCondVar *cond);

void os_condvar_destroy(OSSyncCondVar *cond);

int os_condvar_timedwait(OSSyncCondVar *cond, OSSyncMutex *mutex,
                         int timeout_ms);

void os_condvar_broadcast(OSSyncCondVar *cond);

void os_condvar_signal(OSSyncCondVar *cond);

#define OS_WAIT_SUCCESS 0
#define OS_WAIT_TIMEOUT 1
#define OS_WAIT_ERROR -1

#endif  // SYNC_H