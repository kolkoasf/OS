#include "mutex.h"

int CreateMutex(mutex_t* mutex) {
  int res = pthread_mutex_init(mutex, NULL);
  if (res != 0) {
    errno = res;
    return -1;
  }
  return 0;
}

int LockMutex(mutex_t* mutex) {
  int res = pthread_mutex_lock(mutex);
  if (res != 0) {
    errno = res;
    return -1;
  }
  return 0;
}

int UnlockMutex(mutex_t* mutex) {
  int res = pthread_mutex_unlock(mutex);
  if (res != 0) {
    errno = res;
    return -1;
  }
  return 0;
}

int DestroyMutex(mutex_t* mutex) {
  int res = pthread_mutex_destroy(mutex);
  if (res != 0) {
    errno = res;
    return -1;
  }
  return 0;
}