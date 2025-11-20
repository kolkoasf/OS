#include "helpers.h"

#include <stdio.h>
#include <string.h>

ipc_handles_t InitIpcHandles(int child_id, const char *input_path,
                             const char *data_ready_name,
                             const char *processed_name) {
  ipc_handles_t handles = {0};
  handles.data_ready_name = data_ready_name;
  handles.processed_name = processed_name;

  handles.input_fd = CreateMappedFile(input_path, sizeof(shared_buffer_t));
  if (handles.input_fd == -1) {
    fprintf(stderr, "[Parent] Ошибка создания входного файла для child%d\n",
            child_id);
    return handles;
  }

  handles.input_mmap = MapFileToMemory(
      handles.input_fd, sizeof(shared_buffer_t), PROT_READ | PROT_WRITE);
  if (handles.input_mmap.addr == NULL) {
    fprintf(stderr, "[Parent] Ошибка отображения входного файла для child%d\n",
            child_id);
    CloseObject(handles.input_fd);
    handles.data_ready = NULL;
    return handles;
  }

  handles.data_ready = CreateNamedSemaphore(data_ready_name, 0);
  handles.processed = CreateNamedSemaphore(processed_name, 0);

  if (handles.data_ready == NULL || handles.processed == NULL) {
    fprintf(stderr, "[Parent] Ошибка создания семафоров для child%d\n",
            child_id);
    UnmapFile(handles.input_mmap);
    CloseObject(handles.input_fd);

    if (handles.data_ready) {
      CloseSemaphore(handles.data_ready);
    }
    if (handles.processed) {
      CloseSemaphore(handles.processed);
    }
    handles.data_ready = NULL;
    return handles;
  }

  shared_buffer_t *buf = (shared_buffer_t *)handles.input_mmap.addr;
  memset(buf, 0, sizeof(shared_buffer_t));

  return handles;
}

void CleanupChildIpc(ipc_handles_t *handles) {
  if (handles == NULL) {
    return;
  }

  UnmapFile(handles->input_mmap);
  CloseObject(handles->input_fd);
  CloseSemaphore(handles->data_ready);
  CloseSemaphore(handles->processed);
  UnlinkSemaphore(handles->data_ready_name);
  UnlinkSemaphore(handles->processed_name);
}

int SendDataToChild(shared_buffer_t *buf, const char *data, size_t len,
                    semaphore_t sem_ready, semaphore_t sem_processed) {
  if (len > MAX_LINE_LENGTH - 1) {
    fprintf(stderr, "[Parent] Ошибка: строка слишком длинная\n");
    return -1;
  }

  memcpy(buf->data, data, len + 1);
  buf->line_length = len;
  buf->shutdown_flag = 0;

  if (PostSemaphore(sem_ready) == -1) {
    return -1;
  }

  if (WaitSemaphore(sem_processed) == -1) {
    return -1;
  }

  return 0;
}

void SignalChildShutdown(shared_buffer_t *buf, semaphore_t sem_ready) {
  buf->shutdown_flag = 1;
  PostSemaphore(sem_ready);
}

void WaitForChildren(process_id_t pid1, process_id_t pid2, int *status1,
                     int *status2) {
  *status1 = 0;
  *status2 = 0;
  WaitObject(pid1, status1, 0);
  WaitObject(pid2, status2, 0);
}

int InitChildConfig(int argc, char *argv[], child_config_t *config) {
  if (argc != 4) {
    fprintf(stderr, "Child: неверное количество аргументов\n");
    fprintf(stderr,
            "Usage: %s <child_id> <input_mmap_path> <output_file_path>\n",
            argv[0]);
    return -1;
  }

  config->id = atoi(argv[1]);
  config->input_mmap_path = argv[2];
  config->output_file_path = argv[3];

  return 0;
}

ipc_handles_t SetupChildIpc(const child_config_t *config) {
  ipc_handles_t handles;
  memset(&handles, 0, sizeof(handles));

  const char *data_ready_name =
      (config->id == 1) ? "/sem_child1_data_ready" : "/sem_child2_data_ready";
  const char *processed_name =
      (config->id == 1) ? "/sem_child1_processed" : "/sem_child2_processed";

  handles.data_ready_name = data_ready_name;
  handles.processed_name = processed_name;

  handles.data_ready = OpenNamedSemaphore(data_ready_name);
  handles.processed = OpenNamedSemaphore(processed_name);

  if (handles.data_ready == NULL || handles.processed == NULL) {
    fprintf(stderr, "[Child%d] Ошибка открытия семафоров\n", config->id);
    return handles;
  }

  handles.input_fd = OpenObject(config->input_mmap_path, O_RDWR, 0666);
  if (handles.input_fd == -1) {
    fprintf(stderr, "[Child%d] Ошибка открытия входного файла\n", config->id);
    CloseSemaphore(handles.data_ready);
    CloseSemaphore(handles.processed);
    handles.data_ready = NULL;
    return handles;
  }

  handles.input_mmap = MapFileToMemory(
      handles.input_fd, sizeof(shared_buffer_t), PROT_READ | PROT_WRITE);
  if (handles.input_mmap.addr == NULL) {
    fprintf(stderr, "[Child%d] Ошибка отображения входного файла\n",
            config->id);
    CloseObject(handles.input_fd);
    CloseSemaphore(handles.data_ready);
    CloseSemaphore(handles.processed);
    handles.data_ready = NULL;
    return handles;
  }

  return handles;
}

void CleanupChildIpcSafe(ipc_handles_t *handles) {
  if (handles == NULL) {
    return;
  }

  UnmapFile(handles->input_mmap);
  CloseObject(handles->input_fd);
  CloseSemaphore(handles->data_ready);
  CloseSemaphore(handles->processed);
}

int CreateOutputFile(const child_config_t *config) {
  int fd =
      OpenObject(config->output_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (fd == -1) {
    fprintf(stderr, "[Child%d] Ошибка создания выходного файла %s\n",
            config->id, config->output_file_path);
    return -1;
  }
  return fd;
}

int SignalReady(const child_config_t *config) {
  int sig = (config->id == 1) ? SIGUSR1 : SIGUSR2;
  return SendSignal(GetParentProcessId(), sig);
}

int WriteToOutput(int fd, const char *data, size_t data_len, int child_id) {
  ssize_t written = WriteToObject(fd, data, data_len);
  if (written == -1 || (size_t)written != data_len) {
    fprintf(stderr, "[Child%d] Ошибка записи в выходной файл\n", child_id);
    return -1;
  }
  return 0;
}