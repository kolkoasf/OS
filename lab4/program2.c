#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_parser.h"
#include "contract.h"
#include "dynamic_loader.h"

#define MAX_LIBS 2

typedef int (*PrimeCountFunc)(int, int);
typedef char* (*TranslateFunc)(long);

typedef struct {
  DynamicLib libs[MAX_LIBS];
  PrimeCountFunc prime_funcs[MAX_LIBS];
  TranslateFunc translate_funcs[MAX_LIBS];
  int current_impl;
} LibSet;

static LibSet primes = {{0}, {0}, {0}, 0};
static LibSet translates = {{0}, {0}, {0}, 0};

static void switch_impl_primes(void) {
  int new_impl = 1 - primes.current_impl;
  if (primes.prime_funcs[new_impl]) {
    primes.current_impl = new_impl;
    printf("Switched PrimeCount to impl %d (%s)\n\n", new_impl + 1,
           new_impl == 0 ? "Naive" : "Sieve");
  } else {
    fprintf(stderr, "Error: Alternative implementation not available\n");
  }
}

static void switch_impl_translates(void) {
  int new_impl = 1 - translates.current_impl;
  if (translates.translate_funcs[new_impl]) {
    translates.current_impl = new_impl;
    printf("Switched Translate to impl %d (%s)\n\n", new_impl + 1,
           new_impl == 0 ? "Binary" : "Ternary");
  } else {
    fprintf(stderr, "Error: Alternative implementation not available\n");
  }
}

static void handle_cmd_0(void) {
  switch_impl_primes();
  switch_impl_translates();
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

  if (!primes.prime_funcs[primes.current_impl]) {
    fprintf(stderr, "Error: PrimeCount not loaded\n");
    return;
  }

  int result = primes.prime_funcs[primes.current_impl](A, B);
  printf("PrimeCount(%d, %d) = %d\n", A, B, result);
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

  if (!translates.translate_funcs[translates.current_impl]) {
    fprintf(stderr, "Error: Translation function not loaded\n");
    return;
  }

  char* result = translates.translate_funcs[translates.current_impl](num);

  if (result) {
    printf("Decimal: %ld\n", num);
    if (translates.current_impl == 0) {
      printf("Binary:  %s\n", result);
    } else {
      printf("Ternary: %s\n", result);
    }
    free(result);
  } else {
    fprintf(stderr, "Error: Translation failed\n");
  }
}

int main(void) {
  char path_primes[256];
  char path_translates[256];
  snprintf(path_primes, sizeof(path_primes), "%slibprimes%s", LIB_PATH_PREFIX,
           LIB_EXT);
  snprintf(path_translates, sizeof(path_translates), "%slibtranslation%s",
           LIB_PATH_PREFIX, LIB_EXT);

  DynamicLib lib_primes_handle = open_library(path_primes);
  if (!lib_primes_handle) {
    fprintf(stderr, "Failed to load libprimes: %s\n", get_last_error());
    return 1;
  }

  DynamicLib lib_translates_handle = open_library(path_translates);
  if (!lib_translates_handle) {
    fprintf(stderr, "Failed to load libtranslation: %s\n", get_last_error());
    close_library(lib_primes_handle);
    return 1;
  }

  primes.libs[0] = lib_primes_handle;
  primes.libs[1] = lib_primes_handle;
  translates.libs[0] = lib_translates_handle;
  translates.libs[1] = lib_translates_handle;

  primes.prime_funcs[0] =
      (PrimeCountFunc)get_symbol_library(lib_primes_handle, "PrimeCount");
  primes.prime_funcs[1] =
      (PrimeCountFunc)get_symbol_library(lib_primes_handle, "PrimeCount_Sieve");

  translates.translate_funcs[0] = (TranslateFunc)get_symbol_library(
      lib_translates_handle, "Translate_Binary");
  translates.translate_funcs[1] = (TranslateFunc)get_symbol_library(
      lib_translates_handle, "Translate_Ternary");

  if (!primes.prime_funcs[0] || !primes.prime_funcs[1] ||
      !translates.translate_funcs[0] || !translates.translate_funcs[1]) {
    fprintf(stderr, "Failed to load symbols: %s\n", get_last_error());
    close_library(lib_primes_handle);
    close_library(lib_translates_handle);
    return 1;
  }

  printf("\n=== Program 2 (Runtime Dynamic Loading) ===\n");
  printf(
      "Commands: 0 | 1 A B | 2 number | help | "
      "exit\n\n");

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
      printf("0: Switch implementations (Binary <-> Ternary)\n");
      printf("1 A B: PrimeCount in range [A,B]\n");
      printf("2 number: Translate decimal (currently: %s)\n",
             translates.current_impl == 0 ? "Binary" : "Ternary");
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

  close_library(lib_primes_handle);
  close_library(lib_translates_handle);

  printf("Goodbye!\n");
  return 0;
}
