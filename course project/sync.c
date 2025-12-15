#include "sync.h"

#include <stdio.h>

#include "sync_init.h"

#ifdef _WIN32
#else
#include <errno.h>
#include <pthread.h>
#include <time.h>
#endif

int os_mutex_init(OSSyncMutex *mutex) {
  pthread_mutexattr_t attr;

  if (pthread_mutexattr_init(&attr) != 0) {
    perror("pthread_mutexattr_init");
    return -1;
  }

  if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
    perror("pthread_mutexattr_setpshared");
    pthread_mutexattr_destroy(&attr);
    return -1;
  }

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
    perror("pthread_mutexattr_settype");
    pthread_mutexattr_destroy(&attr);
    return -1;
  }

  int result = pthread_mutex_init(mutex, &attr);
  pthread_mutexattr_destroy(&attr);
  return result;
}

void os_mutex_destroy(OSSyncMutex *mutex) {
  if (mutex) {
    pthread_mutex_destroy(mutex);
  }
}

void os_mutex_lock(OSSyncMutex *mutex) {
  if (mutex) {
    pthread_mutex_lock(mutex);
  }
}

void os_mutex_unlock(OSSyncMutex *mutex) {
  if (mutex) {
    pthread_mutex_unlock(mutex);
  }
}

int os_condvar_init(OSSyncCondVar *cond) {
  pthread_condattr_t attr;

  if (pthread_condattr_init(&attr) != 0) {
    perror("pthread_condattr_init");
    return -1;
  }

  if (pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
    perror("pthread_condattr_setpshared");
    pthread_condattr_destroy(&attr);
    return -1;
  }

  int result = pthread_cond_init(cond, &attr);
  pthread_condattr_destroy(&attr);
  return result;
}

void os_condvar_destroy(OSSyncCondVar *cond) {
  if (cond) {
    pthread_cond_destroy(cond);
  }
}

int os_condvar_timedwait(OSSyncCondVar *cond, OSSyncMutex *mutex,
                         int timeout_ms) {
  if (!cond || !mutex) {
    return OS_WAIT_ERROR;
  }

  struct timespec timeout;
  if (clock_gettime(CLOCK_REALTIME, &timeout) != 0) {
    perror("clock_gettime");
    return OS_WAIT_ERROR;
  }

  long ns = (long)timeout.tv_nsec + ((long)timeout_ms * 1000000);
  timeout.tv_sec += ns / 1000000000;
  timeout.tv_nsec = ns % 1000000000;

  int result = pthread_cond_timedwait(cond, mutex, &timeout);

  if (result == ETIMEDOUT) {
    return OS_WAIT_TIMEOUT;
  } else if (result == 0) {
    return OS_WAIT_SUCCESS;
  } else {
    return OS_WAIT_ERROR;
  }
}

void os_condvar_broadcast(OSSyncCondVar *cond) {
  if (cond) {
    pthread_cond_broadcast(cond);
  }
}

void os_condvar_signal(OSSyncCondVar *cond) {
  if (cond) {
    pthread_cond_signal(cond);
  }
}

int init_server_mutexes(ServerState *state) {
  if (!state) return 0;

  printf("Initialized state_mutex\n");
  if (os_mutex_init(&state->state_mutex) != 0) {
    return 0;
  }

  printf("Initialized state_changed condition variable\n");
  if (os_condvar_init(&state->state_changed) != 0) {
    os_mutex_destroy(&state->state_mutex);
    return 0;
  }

  return 1;
}

int init_game_mutexes(GameState *game) {
  if (!game) return 0;

  if (os_mutex_init(&game->game_mutex) != 0) {
    return 0;
  }

  if (os_condvar_init(&game->shot_changed) != 0) {
    os_mutex_destroy(&game->game_mutex);
    return 0;
  }

  return 1;
}

void cleanup_server_mutexes(ServerState *state) {
  if (!state) return;

  for (int i = 0; i < state->game_count; i++) {
    os_mutex_destroy(&state->games[i].game_mutex);
    os_condvar_destroy(&state->games[i].shot_changed);
  }

  os_mutex_destroy(&state->state_mutex);
  os_condvar_destroy(&state->state_changed);

  printf("Synchronization primitives cleaned up\n");
}
