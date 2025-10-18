#include "os.h"

void Reverse(char* line, size_t n) {
  int is_new_line = 0;
  if (n > 0 && line[n - 1] == '\n') {
    is_new_line = 1;
    n--;
  }

  size_t i = 0;
  size_t j = 0;
  if (n > 0) {
    j = n - 1;
  }

  while (i < j) {
    char temp = line[i];
    line[i] = line[j];
    line[j] = temp;
    j--;
    i++;
  }

  if (is_new_line == 1) {
    line[n] = '\n';
    line[n + 1] = '\0';
  } else {
    line[n] = '\0';
  }
}

int main() {
  char* line = NULL;
  size_t capacity = 0;
  ssize_t s;

  while ((s = DoGetline(&line, &capacity, stdin)) != -1) {
    Reverse(line, (size_t)s);
    if (fwrite(line, 1, strlen(line), stdout) != strlen(line)) {
      perror("error while write in file");
      TerminateProc(1);
    }
    fflush(stdout);
  }

  free(line);

  return 0;
}
