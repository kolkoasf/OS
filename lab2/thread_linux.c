#include "thread.h"

void ThreadInit(Thread* t, ThreadFunc func) {
  t->func = func;
  t->thread = 0;
}

int ThreadJoin(Thread* t) {
  int res = pthread_join(t->thread, NULL);
  if (res != 0) {
    errno = res;
  }
  return res;
}

int ThreadDetach(Thread* t) {
  int res = pthread_detach(t->thread);
  if (res != 0) {
    errno = res;
  }
  return res;
}

int ThreadRun(Thread* t, void* arg) {
  int res = pthread_create(&t->thread, NULL, t->func, arg);
  if (res != 0) {
    errno = res;
  }
  return res;
}
