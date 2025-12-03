#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_parser.h"
#include "contract.h"

typedef struct {
  int argc;
  char** argv;
} Args;

static void handle_cmd_1(Args args) {
  if (args.argc < 3) {
    fprintf(stderr, "Error: PrimeCount needs 2 args: A B\n");
    return;
  }

  int error_a, error_b;
  int A = safe_strtol(args.argv[1], &error_a);
  int B = safe_strtol(args.argv[2], &error_b);

  if (error_a) {
    fprintf(stderr, "Error: Invalid argument A '%s' - not a valid integer\n",
            args.argv[1]);
    return;
  }

  if (error_b) {
    fprintf(stderr, "Error: Invalid argument B '%s' - not a valid integer\n",
            args.argv[2]);
    return;
  }

  int result = PrimeCount(A, B);
  printf("PrimeCount(%d, %d) = %d\n", A, B, result);
}

static void handle_cmd_2(Args args) {
  if (args.argc < 2) {
    fprintf(stderr, "Error: Translate needs 1 arg: number\n");
    return;
  }

  int error;
  long num = strtol(args.argv[1], NULL, 10);
  if (error) {
    fprintf(stderr, "Error: Invalid number '%s'\n", args.argv[1]);
    return;
  }

  char* binary = Translate_Binary(num);
  char* ternary = Translate_Ternary(num);

  if (binary) {
    printf("Decimal: %ld\n", num);
    printf("Binary:  %s\n", binary);
    free(binary);
  }
  if (ternary) {
    printf("Ternary: %s\n", ternary);
    free(ternary);
  }
}

int main(void) {
  printf("\n=== Program 1 (Compile-time Linking) ===\n");
  printf("Commands: 1 A B | 2 number | help | exit\n\n");

  char line[BUFFER_SIZE];
  while (read_line(line)) {
    if (strlen(line) == 0) {
      continue;
    }

    Args args;
    int argc = parse_line(line, &args.argv);
    if (argc <= 0) {
      continue;
    }

    args.argc = argc;

    if (!strcmp(args.argv[0], "exit")) {
      free_argv(argc, args.argv);
      break;
    } else if (!strcmp(args.argv[0], "help")) {
      printf("1 A B: PrimeCount in range [A,B]\n");
      printf("2 number: Translate decimal to binary and ternary\n");
    } else if (!strcmp(args.argv[0], "1")) {
      handle_cmd_1(args);
    } else if (!strcmp(args.argv[0], "2")) {
      handle_cmd_2(args);
    } else {
      printf("Unknown: %s\n", args.argv[0]);
    }

    free_argv(argc, args.argv);
  }

  printf("Goodbye!\n");
  return 0;
}
