#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256
#define MAX_ARRAY_SIZE 1000

static inline void print_translation(const char* result, const char* prefix) {
  printf("%s%s\n", prefix, result);
}

static inline int safe_strtol(const char* str, int* error) {
  char* endptr;
  errno = 0;

  long val = strtol(str, &endptr, 10);

  if (errno != 0) {
    *error = 1;
    return 0;
  }

  if (*endptr != '\0') {
    *error = 1;
    return 0;
  }

  if (val > INT_MAX || val < INT_MIN) {
    *error = 1;
    return 0;
  }

  *error = 0;
  return (int)val;
}

static inline int parse_line(const char* line, char*** argv_ptr) {
  char* copy = (char*)malloc(strlen(line) + 1);
  if (!copy) return 0;
  strcpy(copy, line);

  *argv_ptr = (char**)malloc(sizeof(char*) * (MAX_ARRAY_SIZE + 2));
  if (!*argv_ptr) {
    free(copy);
    return 0;
  }

  int argc = 0;
  char* token = strtok(copy, " ");
  while (token && argc < MAX_ARRAY_SIZE + 1) {
    (*argv_ptr)[argc] = (char*)malloc(strlen(token) + 1);
    if (!(*argv_ptr)[argc]) {
      free(copy);
      return -1;
    }
    strcpy((*argv_ptr)[argc], token);
    argc++;
    token = strtok(NULL, " ");
  }

  free(copy);
  return argc;
}

static inline void free_argv(int argc, char** argv) {
  if (!argv) return;
  for (int i = 0; i < argc; i++)
    if (argv[i]) free(argv[i]);
  free(argv);
}

static inline char* read_line(char line[BUFFER_SIZE]) {
  if (!fgets(line, BUFFER_SIZE, stdin)) return NULL;
  size_t len = strlen(line);
  if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
  return line;
}
