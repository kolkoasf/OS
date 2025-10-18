#include "os.h"

void WriteToPipe(pipe_t fd, const char *line, size_t n) {
  size_t written = 0;

  while (written < n) {
    ssize_t s = PipeWrite(fd, line + written, n - written);

    if (s == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("error while write to pipe");
      TerminateProc(1);
    }

    written += (size_t)s;
  }
}

int main(int argc, char *argv[], char *envp[]) {
  char file_odd[4096];
  char file_even[4096];
  printf("Enter a file name for odd lines: ");
  fgets(file_odd, sizeof(file_odd), stdin);
  printf("Enter a file name for even lines: ");
  fgets(file_even, sizeof(file_even), stdin);

  file_odd[strcspn(file_odd, "\n")] = 0;
  file_even[strcspn(file_even, "\n")] = 0;

  pipe_t fd_odd = OpenObject(file_odd, O_CREAT | O_TRUNC | O_WRONLY, 00666);
  if (fd_odd == -1) {
    perror("error while create/open file for odd");
    TerminateProc(1);
  }
  pipe_t fd_even = OpenObject(file_even, O_CREAT | O_TRUNC | O_WRONLY, 00666);
  if (fd_even == -1) {
    perror("error while create/open file for even");
    TerminateProc(1);
  }

  pipe_t p_odd[2];
  pipe_t p_even[2];
  if (CreatePipe(p_odd) == -1) {
    perror("error while create pipe for odd");
    TerminateProc(1);
  }
  if (CreatePipe(p_even) == -1) {
    perror("error while create pipe for even");
    TerminateProc(1);
  }

  char *args[] = {"child", NULL};

  proc_info_t child_odd = CreateProc("./child", args, envp, p_odd[0], fd_odd);
  if (child_odd.pid == -1) {
    perror("error while create odd child process");
    TerminateProc(1);
  }

  proc_info_t child_even =
      CreateProc("./child", args, envp, p_even[0], fd_even);
  if (child_even.pid == -1) {
    perror("error while create even child process");
    TerminateProc(1);
  }

  char *line = NULL;
  int line_num = 1;
  size_t capacity = 0;
  ssize_t s;

  while ((s = DoGetline(&line, &capacity, stdin)) != -1) {
    if (line_num % 2 != 0) {
      WriteToPipe(p_odd[1], line, (size_t)s);
    } else {
      WriteToPipe(p_even[1], line, (size_t)s);
    }
    line_num++;
  }

  CloseObject(p_odd[1]);
  CloseObject(p_even[1]);

  int status;
  if (WaitObject(child_odd, &status, 0) == -1) {
    perror("error in waitpid child_odd");
    TerminateProc(1);
  }
  if (WaitObject(child_even, &status, 0) == -1) {
    perror("error in waitpid child_even");
    TerminateProc(1);
  }

  free(line);
  return 0;
}
