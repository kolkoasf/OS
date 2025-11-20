#pragma once

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _GNU_SOURCE

#ifdef _WIN32

#else
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef int pipe_t;
typedef pid_t process_id_t;

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

void* MapSharedMemory(int fd, size_t size);

int UnmapMemory(void* addr, size_t size);

int CreateSharedMemory(const char* name, size_t size);

int OpenSharedMemory(const char* name);

int UnlinkSharedMemory(const char* name);

semaphore_t CreateNamedSemaphore(const char* name, int initial_value);

semaphore_t OpenNamedSemaphore(const char* name);

int WaitSemaphore(semaphore_t sem);

int PostSemaphore(semaphore_t sem);

int CloseSemaphore(semaphore_t sem);

int UnlinkSemaphore(const char* name);