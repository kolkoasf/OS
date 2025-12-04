#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_parser.h"
#include "contract.h"

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

  int result = PrimeCount(A, B);
  printf("PrimeCount(%d, %d) = %d (naive)\n", A, B, result);
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

  char* result = translation(num);

  printf("Decimal: %ld\n", num);

  if (result) {
    printf("Binary: %s\n", result);
    free(result);
  }
}

int main(void) {
  printf("\n=== Program 1 (Static Linking - Light Implementation) ===\n");
  printf("Commands: 1 A B | 2 number | help | exit\n\n");

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
      printf("1 A B: PrimeCount (naive) in range [A,B]\n");
      printf("2 number: Translate decimal to binary\n");
      printf("exit: Exit program\n");
    } else if (!strcmp(argv[0], "1")) {
      handle_cmd_1(argc, argv);
    } else if (!strcmp(argv[0], "2")) {
      handle_cmd_2(argc, argv);
    } else {
      printf("Unknown: %s\n", argv[0]);
    }

    free_argv(argc, argv);
  }

  printf("Goodbye!\n");
  return 0;
}