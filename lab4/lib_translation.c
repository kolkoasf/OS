#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contract.h"

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

char* Translate_Binary(long x) {
  if (x == 0) {
    char* result = (char*)malloc(2);
    if (result) {
      strcpy(result, "0");
    }
    return result;
  }

  return translate_to_base(x, 2);
}

char* Translate_Ternary(long x) {
  if (x == 0) {
    char* result = (char*)malloc(2);
    if (result) {
      strcpy(result, "0");
    }
    return result;
  }

  return translate_to_base(x, 3);
}
