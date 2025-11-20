#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _GNU_SOURCE

#ifdef _WIN32

#else
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef int pipe_t;
typedef pid_t process_id_t;
typedef void* mmap_handle_t;
typedef void (*signal_handler_t)(int);

typedef struct {
  void* addr;
  size_t size;
  int fd;
} mmap_info_t;

typedef sem_t* semaphore_t;

#endif

process_id_t CreateProc(const char* file, char* argv[], char* envp[]);

int WaitObject(process_id_t proc_info, int* status, int options);

void TerminateProc(int status);

process_id_t GetProcessId(void);

process_id_t GetParentProcessId(void);

pipe_t OpenObject(const char* path, int flags, int mode);

int CloseObject(pipe_t fd);

ssize_t WriteToObject(pipe_t fd, const void* line, size_t count);

int TruncateFile(pipe_t fd, off_t size);

pipe_t CreateMappedFile(const char* path, size_t size);

mmap_info_t MapFileToMemory(pipe_t fd, size_t size, int prot);

int UnmapFile(mmap_info_t info);

int SyncMappedFile(mmap_info_t info);

int RegisterSignalHandler(int sig, signal_handler_t handler);

int SendSignal(process_id_t pid, int sig);

semaphore_t CreateNamedSemaphore(const char* name, int initial_value);

semaphore_t OpenNamedSemaphore(const char* name);

int WaitSemaphore(semaphore_t sem);

int PostSemaphore(semaphore_t sem);

int CloseSemaphore(semaphore_t sem);

int UnlinkSemaphore(const char* name);