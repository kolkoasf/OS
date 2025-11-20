#include "os.h"

process_id_t CreateProc(const char* file, char* argv[], char* envp[]) {
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    execve(file, argv, envp);
    perror("execve");
    TerminateProc(EXIT_FAILURE);
  }

  return pid;
}

int WaitObject(process_id_t proc_id, int* status, int options) {
  int res = waitpid(proc_id, status, options);
  if (res == -1) {
    perror("waitpid");
  }
  return res;
}

void TerminateProc(int status) { _exit(status); }

process_id_t GetProcessId(void) { return getpid(); }

process_id_t GetParentProcessId(void) { return getppid(); }

pipe_t OpenObject(const char* path, int flags, int mode) {
  int fd = open(path, flags, mode);
  if (fd == -1) {
    perror("open");
  }
  return fd;
}

int CloseObject(pipe_t fd) {
  if (fd < 0) return 0;

  int res = close(fd);
  if (res == -1) {
    perror("close");
  }
  return res;
}

ssize_t WriteToObject(pipe_t fd, const void* data, size_t count) {
  return write(fd, data, count);
}

void* MapSharedMemory(int fd, size_t size) {
  void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    perror("mmap");
    return NULL;
  }
  return addr;
}

int UnmapMemory(void* addr, size_t size) {
  if (addr == NULL) return 0;

  int res = munmap(addr, size);
  if (res == -1) {
    perror("munmap");
  }
  return res;
}

int CreateSharedMemory(const char* name, size_t size) {
  shm_unlink(name);

  int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, 0666);
  if (fd == -1) {
    perror("shm_open");
    return -1;
  }

  if (ftruncate(fd, size) == -1) {
    perror("ftruncate");
    close(fd);
    shm_unlink(name);
    return -1;
  }

  return fd;
}

int OpenSharedMemory(const char* name) {
  int fd = shm_open(name, O_RDWR, 0);
  if (fd == -1) {
    perror("shm_open");
  }
  return fd;
}

int UnlinkSharedMemory(const char* name) {
  int res = shm_unlink(name);
  if (res == -1) {
    perror("shm_unlink");
  }
  return res;
}

semaphore_t CreateNamedSemaphore(const char* name, int initial_value) {
  sem_unlink(name);

  semaphore_t sem = sem_open(name, O_CREAT | O_EXCL, 0666, initial_value);
  if (sem == SEM_FAILED) {
    perror("sem_open");
    return NULL;
  }
  return sem;
}

semaphore_t OpenNamedSemaphore(const char* name) {
  semaphore_t sem = sem_open(name, 0);
  if (sem == SEM_FAILED) {
    perror("sem_open");
    return NULL;
  }
  return sem;
}

int WaitSemaphore(semaphore_t sem) {
  if (sem == NULL) {
    fprintf(stderr, "WaitSemaphore: NULL semaphore\n");
    return -1;
  }

  int res = sem_wait(sem);
  if (res == -1) {
    perror("sem_wait");
  }
  return res;
}

int PostSemaphore(semaphore_t sem) {
  if (sem == NULL) {
    fprintf(stderr, "PostSemaphore: NULL semaphore\n");
    return -1;
  }

  int res = sem_post(sem);
  if (res == -1) {
    perror("sem_post");
  }
  return res;
}

int CloseSemaphore(semaphore_t sem) {
  if (sem == NULL) return 0;

  int res = sem_close(sem);
  if (res == -1) {
    perror("sem_close");
  }
  return res;
}

int UnlinkSemaphore(const char* name) {
  int res = sem_unlink(name);
  if (res == -1) {
    perror("sem_unlink");
  }
  return res;
}