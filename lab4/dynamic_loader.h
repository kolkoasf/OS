#include <stdio.h>

#ifdef _WIN32

#define LIB_EXT ".dll"
#define LIB_PATH_PREFIX "./lib/"

#else

#include <dlfcn.h>

typedef void* DynamicLib;

static inline DynamicLib open_library(const char* path) {
  return dlopen(path, RTLD_LAZY);
}

static inline void* get_symbol_library(DynamicLib lib, const char* symbol) {
  return dlsym(lib, symbol);
}

static inline int close_library(DynamicLib lib) {
  return dlclose(lib) == 0 ? 1 : 0;
}

static inline const char* get_last_error(void) { return dlerror(); }

#define LIB_EXT ".so"
#define LIB_PATH_PREFIX "./lib/"

#endif
