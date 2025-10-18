#include <errno.h>

#ifdef _WIN32
#else
#include <pthread.h>
#include <unistd.h>
#endif

typedef void* (*ThreadFunc)(void*);

typedef struct {
  ThreadFunc func;
  pthread_t thread;
} Thread;

void ThreadInit(Thread* t, ThreadFunc func);

int ThreadJoin(Thread* t);

int ThreadDetach(Thread* t);

int ThreadRun(Thread* t, void* arg);
