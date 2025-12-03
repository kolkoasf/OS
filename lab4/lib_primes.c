#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contract.h"

static int count_primes_naive(int A, int B) {
  if (A > B) {
    return 0;
  }
  if (B < 2) {
    return 0;
  }

  if (A < 2) {
    A = 2;
  }

  int count = 0;

  for (int num = A; num <= B; num++) {
    int is_prime = 1;

    for (int i = 2; i * i <= num; i++) {
      if (num % i == 0) {
        is_prime = 0;
        break;
      }
    }

    if (is_prime) {
      count++;
    }
  }

  return count;
}

static int count_primes_sieve(int A, int B) {
  if (B < 2) {
    return 0;
  }

  int start = (A < 2) ? 2 : A;
  int size = B - start + 1;

  unsigned char* is_prime = (unsigned char*)malloc(size);
  if (is_prime == NULL) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    return 0;
  }

  memset(is_prime, 1, size);

  if (start == 0) {
    is_prime[0] = 0;
  }
  if (start == 1) {
    is_prime[1 - start] = 0;
  }

  for (int i = 2; i * i <= B; i++) {
    int first_multiple = ((start + i - 1) / i) * i;

    for (int j = first_multiple; j <= B; j += i) {
      if (j >= start) {
        is_prime[j - start] = 0;
      }
    }
  }

  int count = 0;
  for (int i = 0; i < size; i++) {
    if (is_prime[i]) {
      count++;
    }
  }

  free(is_prime);
  return count;
}

int PrimeCount(int A, int B) {
  if (A > B) {
    return 0;
  }
  if (B < 2) {
    return 0;
  }

  return count_primes_naive(A, B);
}

int PrimeCount_Sieve(int A, int B) {
  if (A > B) {
    return 0;
  }
  if (B < 2) {
    return 0;
  }

  return count_primes_sieve(A, B);
}