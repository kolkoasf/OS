#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_parser.h"
#include "contract.h"
#include "dynamic_loader.h"

#define MAX_LIBS 2

typedef int (*PrimeCountFunc)(int, int);
typedef char* (*TranslationFunc)(long);

typedef struct {
  DynamicLib lib;
  PrimeCountFunc prime_func;
  TranslationFunc translation_func;
} LibImpl;

static LibImpl impl[MAX_LIBS];
static int current_impl = 0;

static void handle_cmd_0(void) {
  int new_impl = 1 - current_impl;

  if (!impl[new_impl].prime_func || !impl[new_impl].translation_func) {
    fprintf(stderr, "Error: Alternative implementation not available\n");
    return;
  }

  current_impl = new_impl;
  printf("Switched to impl %d (%s)\n\n", current_impl + 1,
         current_impl == 0 ? "Light" : "Hard");
}

static void handle_cmd_1(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "Error: PrimeCount needs 2 args: A B\n");
    return;
  }

  int error_a, error_b;
  int A = safe_strtol(argv[1], &error_a);
  int B = safe_strtol(argv[2], &error_b);

  if (error_a) {
    fprintf(stderr, "Error: Invalid argument A '%s' - not a valid integer\n",
            argv[1]);
    return;
  }

  if (error_b) {
    fprintf(stderr, "Error: Invalid argument B '%s' - not a valid integer\n",
            argv[2]);
    return;
  }

  if (!impl[current_impl].prime_func) {
    fprintf(stderr, "Error: PrimeCount not loaded\n");
    return;
  }

  int result = impl[current_impl].prime_func(A, B);
  printf("PrimeCount(%d, %d) = %d (%s)\n", A, B, result,
         current_impl == 0 ? "naive" : "sieve");
}

static void handle_cmd_2(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Error: Translate needs 1 arg: number\n");
    return;
  }

  char* endptr;
  long num = strtol(argv[1], &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Error: Invalid number '%s'\n", argv[1]);
    return;
  }

  if (!impl[current_impl].translation_func) {
    fprintf(stderr, "Error: Translation function not loaded\n");
    return;
  }

  char* result = impl[current_impl].translation_func(num);

  if (result) {
    printf("Decimal: %ld\n", num);
    if (current_impl == 0) {
      printf("Binary: %s\n", result);
    } else {
      printf("Ternary: %s\n", result);
    }
    free(result);
  } else {
    fprintf(stderr, "Error: Translation failed\n");
  }
}

int main(void) {
  char path_light[256];
  char path_hard[256];

  snprintf(path_light, sizeof(path_light), "%sliblight%s", LIB_PATH_PREFIX,
           LIB_EXT);
  snprintf(path_hard, sizeof(path_hard), "%slibhard%s", LIB_PATH_PREFIX,
           LIB_EXT);

  impl[0].lib = open_library(path_light);
  if (!impl[0].lib) {
    fprintf(stderr, "Failed to load lib_light: %s\n", get_last_error());
    return 1;
  }

  impl[1].lib = open_library(path_hard);
  if (!impl[1].lib) {
    fprintf(stderr, "Failed to load lib_hard: %s\n", get_last_error());
    close_library(impl[0].lib);
    return 1;
  }

  impl[0].prime_func =
      (PrimeCountFunc)get_symbol_library(impl[0].lib, "PrimeCount");
  impl[0].translation_func =
      (TranslationFunc)get_symbol_library(impl[0].lib, "translation");

  impl[1].prime_func =
      (PrimeCountFunc)get_symbol_library(impl[1].lib, "PrimeCount");
  impl[1].translation_func =
      (TranslationFunc)get_symbol_library(impl[1].lib, "translation");

  if (!impl[0].prime_func || !impl[0].translation_func || !impl[1].prime_func ||
      !impl[1].translation_func) {
    fprintf(stderr, "Failed to load symbols: %s\n", get_last_error());
    close_library(impl[0].lib);
    close_library(impl[1].lib);
    return 1;
  }

  printf(
      "\n=== Program 2 (Dynamic Loading - Switchable Implementations) ===\n");
  printf("Commands: 0 | 1 A B | 2 number | help | exit\n\n");

  char line[BUFFER_SIZE];
  while (read_line(line)) {
    if (strlen(line) == 0) {
      continue;
    }

    char** argv;
    int argc = parse_line(line, &argv);

    if (argc <= 0) {
      continue;
    }

    if (!strcmp(argv[0], "exit")) {
      free_argv(argc, argv);
      break;
    } else if (!strcmp(argv[0], "help")) {
      printf("0: Switch implementations (Light <-> Hard)\n");
      printf("1 A B: PrimeCount in range [A,B]\n");
      printf("2 number: Translate decimal\n");
      printf("   Current: %s (PrimeCount) + %s (translation)\n",
             current_impl == 0 ? "Light" : "Hard",
             current_impl == 0 ? "Binary" : "Ternary");
      printf("exit: Exit program\n");
    } else if (!strcmp(argv[0], "0")) {
      handle_cmd_0();
    } else if (!strcmp(argv[0], "1")) {
      handle_cmd_1(argc, argv);
    } else if (!strcmp(argv[0], "2")) {
      handle_cmd_2(argc, argv);
    } else {
      printf("Unknown command: %s\n", argv[0]);
    }

    free_argv(argc, argv);
  }

  close_library(impl[0].lib);
  close_library(impl[1].lib);
  printf("Goodbye!\n");
  return 0;
}
