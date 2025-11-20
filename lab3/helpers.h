#pragma once

#include "os.h"

#define MAX_LINE_LENGTH 1024
#define SHM_NAME "/ipc_buffer"
#define SHM_SIZE sizeof(shared_buffer_t)

typedef struct {
  volatile int shutdown_flag;
  volatile int line_length;
  char data[MAX_LINE_LENGTH];
} shared_buffer_t;

typedef struct {
  int shm_fd;
  void* shm_addr;
  semaphore_t sem_parent_write;
  semaphore_t sem_child1_read;
  semaphore_t sem_child2_read;
  semaphore_t sem_ack;
} ipc_handles_t;

typedef struct {
  int id;
  const char* shm_name;
  const char* output_file;
} child_config_t;

ipc_handles_t InitParentIpc(const char* shm_name, size_t shm_size);

ipc_handles_t InitChildIpc(const char* shm_name, size_t shm_size);

void CleanupIpc(ipc_handles_t* handles, size_t shm_size);

void CleanupIpcFull(ipc_handles_t* handles, const char* shm_name,
                    size_t shm_size);

int SendDataToChild(shared_buffer_t* buf, const char* data, size_t len,
                    int child_id, ipc_handles_t* ipc);

void SignalChildShutdown(shared_buffer_t* buf, ipc_handles_t* ipc);

void WaitForChildren(process_id_t pid1, process_id_t pid2, int* status1,
                     int* status2);

int InitChildConfig(int argc, char* argv[], child_config_t* config);

int CreateOutputFile(const child_config_t* config);

int WriteToOutput(int fd, const char* data, size_t data_len);

int CloseOutputFile(int fd);
