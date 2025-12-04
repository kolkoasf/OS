#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contract.h"

static int is_prime(int n) {
  if (n < 2) {
    return 0;
  }
  if (n == 2) {
    return 1;
  }
  if (n % 2 == 0) {
    return 0;
  }

  for (int i = 3; i * i <= n; i += 2) {
    if (n % i == 0) {
      return 0;
    }
  }
  return 1;
}

int PrimeCount(int A, int B) {
  if (A > B) {
    return 0;
  }
  if (A < 2) {
    A = 2;
  }

  int count = 0;
  for (int i = A; i <= B; i++) {
    if (is_prime(i)) {
      count++;
    }
  }
  return count;
}

static char* translate_to_base(long x, int base) {
  if (x == 0) {
    char* result = (char*)malloc(2);
    if (result) {
      strcpy(result, "0");
    }
    return result;
  }

  int is_negative = (x < 0);
  long num = (x < 0) ? -x : x;

  char temp[128];
  int len = 0;
  long temp_num = num;

  while (temp_num > 0) {
    temp_num /= base;
    len++;
  }

  if (is_negative) {
    len++;
  }

  char* result = (char*)malloc(len + 1);
  if (!result) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    return NULL;
  }

  result[len] = '\0';
  temp_num = num;
  int pos = len - 1;

  while (temp_num > 0) {
    int digit = temp_num % base;
    result[pos--] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    temp_num /= base;
  }

  if (is_negative) {
    result[0] = '-';
  }

  return result;
}

char* translation(long x) { return translate_to_base(x, 2); }