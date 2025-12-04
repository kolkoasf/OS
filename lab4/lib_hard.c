#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contract.h"

int PrimeCount(int A, int B) {
  if (A > B) {
    return 0;
  }
  if (A < 2) {
    A = 2;
  }

  int max_num = B;
  if (max_num < 2) {
    return 0;
  }

  char* sieve = (char*)malloc(max_num + 1);
  if (!sieve) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    return 0;
  }

  memset(sieve, 1, max_num + 1);
  sieve[0] = sieve[1] = 0;

  for (int i = 2; i * i <= max_num; i++) {
    if (sieve[i]) {
      for (int j = i * i; j <= max_num; j += i) {
        sieve[j] = 0;
      }
    }
  }

  int count = 0;
  for (int i = A; i <= B; i++) {
    if (sieve[i]) {
      count++;
    }
  }

  free(sieve);
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

char* translation(long x) { return translate_to_base(x, 3); }