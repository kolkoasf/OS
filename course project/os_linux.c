#include "os.h"

#if OS_LINUX

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

OSMmapHandle mmap_create(const char *filename, size_t size) {
  OSMmapHandle handle = {NULL, -1};
  unlink(filename);

  int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
  if (fd < 0) {
    perror("open");
    return handle;
  }

  if (lseek(fd, size - 1, SEEK_SET) == -1) {
    perror("lseek");
    close(fd);
    return handle;
  }

  if (write(fd, "", 1) != 1) {
    perror("write");
    close(fd);
    return handle;
  }

  void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return handle;
  }

  memset(addr, 0, size);
  handle.addr = addr;
  handle.fd = fd;

  return handle;
}

OSMmapHandle mmap_open(const char *filename, size_t size) {
  OSMmapHandle handle = {NULL, -1};

  int fd = open(filename, O_RDWR, 0666);
  if (fd < 0) {
    perror("open");
    return handle;
  }

  void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return handle;
  }

  handle.addr = addr;
  handle.fd = fd;

  return handle;
}

void mmap_close(OSMmapHandle handle, size_t size) {
  if (handle.addr != NULL && handle.addr != MAP_FAILED) {
    munmap(handle.addr, size);
  }

  if (handle.fd >= 0) {
    close(handle.fd);
  }
}

void mmap_write(void *addr, const void *data, size_t size) {
  if (addr && data) {
    memcpy(addr, data, size);
    msync(addr, size, MS_SYNC);
  }
}

void mmap_read(void *addr, void *data, size_t size) {
  if (addr && data) {
    memcpy(data, addr, size);
  }
}

void os_file_unlink(const char *filename) { unlink(filename); }

void init_random(void) { srand((unsigned int)time(NULL) ^ getpid()); }

int os_get_pid(void) { return getpid(); }

void os_usleep(unsigned int microseconds) { usleep(microseconds); }

void os_signal_register_int(OSSignalHandler handler) {
  signal(SIGINT, handler);
}

#endif  // OS_LINUX
