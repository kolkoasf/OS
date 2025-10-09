#pragma once

#include <stddef.h>

#define _GNU_SOURCE

#ifdef _WIN32

#else
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef int pipe_t;
typedef int pid_t;

typedef struct {
  pid_t pid;
} proc_info_t;
#endif

int CreatePipe(pipe_t new_pipe[2]);

proc_info_t CreateProc(const char* file, char* argv[], char* envp[],
                       pipe_t stdin_pipe, pipe_t stdout_pipe);

int DoDup2(pipe_t old_fd, int new_fd);

int CloseObject(pipe_t fd);

int WaitObject(proc_info_t proc_info, int* status, int options);

pipe_t OpenObject(const char* path, int flags, int mode);

ssize_t PipeWrite(pipe_t fd, const void* line, size_t count);

ssize_t PipeRead(pipe_t fd, void* line, size_t count);

ssize_t DoGetline(char** line, size_t* n, FILE* stream);

void TerminateProc(int status);