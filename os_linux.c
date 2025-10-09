#include "os.h"

int CreatePipe(pipe_t new_pipe[2]) { return pipe(new_pipe); }

proc_info_t CreateProc(const char* file, char* argv[], char* envp[],
                       pipe_t stdin_pipe, pipe_t stdout_pipe) {
  proc_info_t result;

  pid_t pid = fork();

  if (pid == -1) {
    result.pid = -1;
    return result;
  }

  if (pid == 0) {
    if (stdin_pipe != -1) {
      if (dup2(stdin_pipe, STDIN_FILENO) == -1) {
        perror("error while dup2 stdin");
        TerminateProc(1);
      }
    }

    if (stdout_pipe != -1) {
      if (dup2(stdout_pipe, STDOUT_FILENO) == -1) {
        perror("error while dup2 stdout");
        TerminateProc(1);
      }
    }

    execve(file, argv, envp);
    perror("error while execve");
    TerminateProc(1);
  }

  result.pid = pid;
  return result;
}

int DoDup2(pipe_t old_fd, int new_fd) { return dup2(old_fd, new_fd); }

int CloseObject(pipe_t fd) { return close(fd); }

int WaitObject(proc_info_t proc_info, int* status, int options) {
  return waitpid(proc_info.pid, status, options);
}

pipe_t OpenObject(const char* path, int flags, int mode) {
  return open(path, flags, mode);
}

ssize_t PipeWrite(pipe_t fd, const void* line, size_t count) {
  return write(fd, line, count);
}

ssize_t PipeRead(pipe_t fd, void* line, size_t count) {
  return read(fd, line, count);
}

ssize_t DoGetline(char** line, size_t* n, FILE* stream) {
  return getline(line, n, stream);
}

void TerminateProc(int status) {
  kill(getpid(), SIGTERM);
  _exit(status);
}
