#ifndef OS_H
#define OS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define OS_WINDOWS 1
#define OS_LINUX 0
#else
#define OS_WINDOWS 0
#define OS_LINUX 1
#endif

typedef struct {
  void *addr;
  int fd;
} OSMmapHandle;

OSMmapHandle mmap_create(const char *filename, size_t size);

OSMmapHandle mmap_open(const char *filename, size_t size);

void mmap_close(OSMmapHandle handle, size_t size);

void mmap_write(void *addr, const void *data, size_t size);

void mmap_read(void *addr, void *data, size_t size);

void os_file_unlink(const char *filename);

void init_random(void);

int os_get_pid(void);

void os_usleep(unsigned int microseconds);

typedef void (*OSSignalHandler)(int);

void os_signal_register_int(OSSignalHandler handler);

#endif  // OS_H