#include "os.h"

process_id_t CreateProc(const char* file, char* argv[], char* envp[]) {
  process_id_t result;
  pid_t pid = fork();

  if (pid == -1) {
    perror("CreateProc: fork() failed");
    result = -1;
    return result;
  }

  if (pid == 0) {
    execve(file, argv, envp);
    perror("CreateProc: execve() failed");
    TerminateProc(EXIT_FAILURE);
  }

  result = pid;
  return result;
}

int WaitObject(process_id_t proc_info, int* status, int options) {
  int res = waitpid(proc_info, status, options);
  if (res == -1) {
    perror("WaitObject: waitpid() failed");
  }
  return res;
}

void TerminateProc(int status) { _exit(status); }

process_id_t GetProcessId(void) { return getpid(); }

process_id_t GetParentProcessId(void) { return getppid(); }

pipe_t OpenObject(const char* path, int flags, int mode) {
  int fd = open(path, flags, mode);
  if (fd == -1) {
    perror("OpenObject: open() failed");
  }
  return fd;
}

int CloseObject(pipe_t fd) {
  if (fd < 0) {
    return 0;
  }
  int res = close(fd);
  if (res == -1) {
    perror("CloseObject: close() failed");
  }
  return res;
}

ssize_t WriteToObject(pipe_t fd, const void* line, size_t count) {
  return write(fd, line, count);
}

int TruncateFile(pipe_t fd, off_t size) {
  int res = ftruncate(fd, size);
  if (res == -1) {
    perror("TruncateFile: ftruncate() failed");
  }
  return res;
}

pipe_t CreateMappedFile(const char* path, size_t size) {
  pipe_t fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fd == -1) {
    perror("CreateMappedFile: open() failed");
    return -1;
  }

  if (ftruncate(fd, size) == -1) {
    perror("CreateMappedFile: ftruncate() failed");
    close(fd);
    return -1;
  }

  return fd;
}

mmap_info_t MapFileToMemory(pipe_t fd, size_t size, int prot) {
  mmap_info_t result;
  result.fd = fd;
  result.size = size;

  result.addr = mmap(NULL, size, prot, MAP_SHARED, fd, 0);
  if (result.addr == MAP_FAILED) {
    perror("MapFileToMemory: mmap() failed");
    result.addr = NULL;
    return result;
  }

  return result;
}

int UnmapFile(mmap_info_t info) {
  if (info.addr == NULL || info.addr == MAP_FAILED) {
    return 0;
  }

  int res = munmap(info.addr, info.size);
  if (res == -1) {
    perror("UnmapFile: munmap() failed");
  }
  return res;
}

int SyncMappedFile(mmap_info_t info) {
  if (info.addr == NULL || info.addr == MAP_FAILED) {
    return -1;
  }

  int res = msync(info.addr, info.size, MS_SYNC);
  if (res == -1) {
    perror("SyncMappedFile: msync() failed");
  }
  return res;
}

int RegisterSignalHandler(int sig, signal_handler_t handler) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = handler;
  sa.sa_flags = 0;

  int res = sigaction(sig, &sa, NULL);
  if (res == -1) {
    perror("RegisterSignalHandler: sigaction() failed");
  }
  return res;
}

int SendSignal(process_id_t pid, int sig) {
  int res = kill(pid, sig);
  if (res == -1) {
    perror("SendSignal: kill() failed");
  }
  return res;
}

semaphore_t CreateNamedSemaphore(const char* name, int initial_value) {
  sem_unlink(name);

  semaphore_t sem = sem_open(name, O_CREAT | O_EXCL, 0666, initial_value);
  if (sem == SEM_FAILED) {
    perror("CreateNamedSemaphore: sem_open() failed");
    return NULL;
  }
  return sem;
}

semaphore_t OpenNamedSemaphore(const char* name) {
  semaphore_t sem = sem_open(name, 0);
  if (sem == SEM_FAILED) {
    perror("OpenNamedSemaphore: sem_open() failed");
    return NULL;
  }
  return sem;
}

int WaitSemaphore(semaphore_t sem) {
  if (sem == NULL) {
    fprintf(stderr, "WaitSemaphore: NULL semaphore");
    return -1;
  }

  int res = sem_wait(sem);
  if (res == -1) {
    perror("WaitSemaphore: sem_wait() failed");
  }
  return res;
}

int PostSemaphore(semaphore_t sem) {
  if (sem == NULL) {
    fprintf(stderr, "PostSemaphore: NULL semaphore");
    return -1;
  }

  int res = sem_post(sem);
  if (res == -1) {
    perror("PostSemaphore: sem_post() failed");
  }
  return res;
}

int CloseSemaphore(semaphore_t sem) {
  if (sem == NULL) {
    return 0;
  }

  int res = sem_close(sem);
  if (res == -1) {
    perror("CloseSemaphore: sem_close() failed");
  }
  return res;
}

int UnlinkSemaphore(const char* name) {
  int res = sem_unlink(name);
  if (res == -1) {
    perror("UnlinkSemaphore: sem_unlink() failed");
  }
  return res;
}