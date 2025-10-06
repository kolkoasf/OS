#pragma once

#include <stddef.h>
#include <sys/types.h>

typedef int pipe_t;

typedef struct {
  pid_t pid;
} proc_info_t;

int CreatePipe(pipe_t new_pipe[2]);

proc_info_t CreateProc(const char* file, char* const argv[], char* const envp[],
                       pipe_t stdin_pipe, pipe_t stdout_pipe);

int DoDup2(pipe_t old_fd, int new_fd);

int CloseObject(pipe_t fd);

int WaitObject(proc_info_t proc_info, int* status, int options);

pipe_t OpenObject(const char* path, int flags, int mode);

ssize_t PipeWrite(pipe_t fd, const void* line, size_t count);

ssize_t PipeRead(pipe_t fd, void* line, size_t count);

ssize_t DoGetline(char** line, size_t* n, FILE* stream);

void TerminateProc(int status);