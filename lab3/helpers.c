#include "helpers.h"

#include <string.h>

ipc_handles_t InitParentIpc(const char* shm_name, size_t shm_size) {
  ipc_handles_t handles = {0};

  handles.shm_fd = CreateSharedMemory(shm_name, shm_size);
  if (handles.shm_fd == -1) {
    fprintf(stderr, "[Parent] Failed to create shared memory\n");
    return handles;
  }

  handles.shm_addr = MapSharedMemory(handles.shm_fd, shm_size);
  if (handles.shm_addr == NULL) {
    fprintf(stderr, "[Parent] Failed to map shared memory\n");
    CloseObject(handles.shm_fd);
    return handles;
  }

  shared_buffer_t* buf = (shared_buffer_t*)handles.shm_addr;
  memset(buf, 0, shm_size);

  handles.sem_parent_write = CreateNamedSemaphore("/sem_parent_write", 1);
  handles.sem_child1_read = CreateNamedSemaphore("/sem_child1_read", 0);
  handles.sem_child2_read = CreateNamedSemaphore("/sem_child2_read", 0);
  handles.sem_ack = CreateNamedSemaphore("/sem_ack", 0);

  if (!handles.sem_parent_write || !handles.sem_child1_read ||
      !handles.sem_child2_read || !handles.sem_ack) {
    fprintf(stderr, "[Parent] Failed to create semaphores\n");
    UnmapMemory(handles.shm_addr, shm_size);
    CloseObject(handles.shm_fd);
    handles.shm_fd = -1;
    return handles;
  }

  return handles;
}

ipc_handles_t InitChildIpc(const char* shm_name, size_t shm_size) {
  ipc_handles_t handles = {0};

  handles.shm_fd = OpenSharedMemory(shm_name);
  if (handles.shm_fd == -1) {
    fprintf(stderr, "[Child] Failed to open shared memory\n");
    return handles;
  }

  handles.shm_addr = MapSharedMemory(handles.shm_fd, shm_size);
  if (handles.shm_addr == NULL) {
    fprintf(stderr, "[Child] Failed to map shared memory\n");
    CloseObject(handles.shm_fd);
    return handles;
  }

  handles.sem_parent_write = OpenNamedSemaphore("/sem_parent_write");
  handles.sem_child1_read = OpenNamedSemaphore("/sem_child1_read");
  handles.sem_child2_read = OpenNamedSemaphore("/sem_child2_read");
  handles.sem_ack = OpenNamedSemaphore("/sem_ack");

  if (!handles.sem_parent_write || !handles.sem_child1_read ||
      !handles.sem_child2_read || !handles.sem_ack) {
    fprintf(stderr, "[Child] Failed to open semaphores\n");
    UnmapMemory(handles.shm_addr, shm_size);
    CloseObject(handles.shm_fd);
    handles.shm_fd = -1;
    return handles;
  }

  return handles;
}

int SendDataToChild(shared_buffer_t* buf, const char* data, size_t len,
                    int child_id, ipc_handles_t* ipc) {
  if (len > MAX_LINE_LENGTH - 1) {
    fprintf(stderr, "[Parent] String too long\n");
    return -1;
  }

  if (WaitSemaphore(ipc->sem_parent_write) == -1) return -1;

  memcpy((void*)buf->data, data, len);
  buf->line_length = len;

  semaphore_t child_sem =
      (child_id == 1) ? ipc->sem_child1_read : ipc->sem_child2_read;

  if (PostSemaphore(child_sem) == -1) {
    PostSemaphore(ipc->sem_parent_write);
    return -1;
  }

  if (WaitSemaphore(ipc->sem_ack) == -1) {
    PostSemaphore(ipc->sem_parent_write);
    return -1;
  }

  buf->line_length = 0;
  PostSemaphore(ipc->sem_parent_write);

  return 0;
}

void SignalChildShutdown(shared_buffer_t* buf, ipc_handles_t* ipc) {
  WaitSemaphore(ipc->sem_parent_write);
  buf->shutdown_flag = 1;
  PostSemaphore(ipc->sem_child1_read);
  PostSemaphore(ipc->sem_child2_read);
}

void WaitForChildren(process_id_t pid1, process_id_t pid2, int* status1,
                     int* status2) {
  *status1 = 0;
  *status2 = 0;
  WaitObject(pid1, status1, 0);
  WaitObject(pid2, status2, 0);
}

int InitChildConfig(int argc, char* argv[], child_config_t* config) {
  if (argc != 4) {
    fprintf(stderr, "Child: incorrect argument count\n");
    return -1;
  }

  config->id = atoi(argv[1]);
  config->shm_name = argv[2];
  config->output_file = argv[3];

  if (config->id != 1 && config->id != 2) {
    fprintf(stderr, "Child: invalid process ID (must be 1 or 2)\n");
    return -1;
  }

  return 0;
}

int CreateOutputFile(const child_config_t* config) {
  int fd = OpenObject(config->output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (fd == -1) {
    fprintf(stderr, "[Child%d] Failed to create output file: %s\n", config->id,
            config->output_file);
  }
  return fd;
}

int WriteToOutput(int fd, const char* data, size_t data_len) {
  ssize_t written = WriteToObject(fd, data, data_len);
  if (written == -1 || (size_t)written != data_len) return -1;
  return 0;
}

int CloseOutputFile(int fd) {
  if (fd < 0) return 0;
  return CloseObject(fd);
}

void CleanupIpc(ipc_handles_t* handles, size_t shm_size) {
  if (handles == NULL) return;
  CloseSemaphore(handles->sem_parent_write);
  CloseSemaphore(handles->sem_child1_read);
  CloseSemaphore(handles->sem_child2_read);
  CloseSemaphore(handles->sem_ack);
  UnmapMemory(handles->shm_addr, shm_size);
  CloseObject(handles->shm_fd);
}

void CleanupIpcFull(ipc_handles_t* handles, const char* shm_name,
                    size_t shm_size) {
  CleanupIpc(handles, shm_size);
  UnlinkSemaphore("/sem_parent_write");
  UnlinkSemaphore("/sem_child1_read");
  UnlinkSemaphore("/sem_child2_read");
  UnlinkSemaphore("/sem_ack");
  UnlinkSharedMemory(shm_name);
}
