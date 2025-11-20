#include <stdio.h>
#include <string.h>

#include "helpers.h"

static void Reverse(char* line) {
  size_t len = strlen(line);
  if (len == 0) {
    return;
  }

  int has_newline = (line[len - 1] == '\n');
  if (has_newline) {
    len--;
  }

  for (size_t i = 0, j = len - 1; i < j; i++, j--) {
    char tmp = line[i];
    line[i] = line[j];
    line[j] = tmp;
  }

  if (has_newline) {
    line[len] = '\n';
    line[len + 1] = '\0';
  } else {
    line[len] = '\0';
  }
}

int main(int argc, char* argv[]) {
  child_config_t config;
  if (InitChildConfig(argc, argv, &config) == -1) {
    return EXIT_FAILURE;
  }

  ipc_handles_t ipc = SetupChildIpc(&config);
  if (ipc.data_ready == NULL) {
    return EXIT_FAILURE;
  }

  int output_fd = CreateOutputFile(&config);
  if (output_fd == -1) {
    CleanupChildIpcSafe(&ipc);
    return EXIT_FAILURE;
  }

  if (SignalReady(&config) == -1) {
    fprintf(stderr, "[Child%d] Ошибка отправки сигнала готовности\n",
            config.id);
    CloseObject(output_fd);
    CleanupChildIpcSafe(&ipc);
    return EXIT_FAILURE;
  }

  char buffer[MAX_LINE_LENGTH];

  while (1) {
    if (WaitSemaphore(ipc.data_ready) == -1) {
      fprintf(stderr, "[Child%d] Ошибка ожидания данных\n", config.id);
      break;
    }

    shared_buffer_t* shared_buf = (shared_buffer_t*)ipc.input_mmap.addr;

    if (shared_buf->shutdown_flag == 1) {
      break;
    }

    int line_len = shared_buf->line_length;
    if (line_len > 0 && line_len < MAX_LINE_LENGTH) {
      memcpy(buffer, shared_buf->data, line_len + 1);
      buffer[line_len] = '\0';

      Reverse(buffer);

      int reversed_len = strlen(buffer);
      if (WriteToOutput(output_fd, buffer, reversed_len, config.id) == -1) {
        PostSemaphore(ipc.processed);
        break;
      }
    }

    if (PostSemaphore(ipc.processed) == -1) {
      fprintf(stderr, "[Child%d] Ошибка сигнализации завершения\n", config.id);
      break;
    }
  }

  CloseObject(output_fd);
  CleanupChildIpcSafe(&ipc);

  return EXIT_SUCCESS;
}