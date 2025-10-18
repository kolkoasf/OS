#pragma once

#ifdef _WIN32
#else
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#endif

typedef pthread_mutex_t mutex_t;

int CreateMutex(mutex_t* mutex);
int LockMutex(mutex_t* mutex);
int UnlockMutex(mutex_t* mutex);
int DestroyMutex(mutex_t* mutex);
