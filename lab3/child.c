#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

static int InvertLine(const char* input, int input_len, char* output) {
  int out_idx = 0;

  int line_end = input_len;
  if (input_len > 0 && input[input_len - 1] == '\n') {
    line_end = input_len - 1;
  }

  for (int i = line_end - 1; i >= 0; i--) {
    output[out_idx++] = input[i];
  }

  if (out_idx < MAX_LINE_LENGTH - 1) {
    output[out_idx++] = '\n';
  }

  return out_idx;
}

int main(int argc, char* argv[]) {
  child_config_t config;
  if (InitChildConfig(argc, argv, &config) == -1) return EXIT_FAILURE;

  ipc_handles_t ipc = InitChildIpc(config.shm_name, SHM_SIZE);
  if (ipc.shm_fd == -1) return EXIT_FAILURE;

  int output_fd = CreateOutputFile(&config);
  if (output_fd == -1) {
    CleanupIpc(&ipc, SHM_SIZE);
    return EXIT_FAILURE;
  }

  shared_buffer_t* shared_buf = (shared_buffer_t*)ipc.shm_addr;

  semaphore_t sem_read =
      (config.id == 1) ? ipc.sem_child1_read : ipc.sem_child2_read;

  while (1) {
    if (WaitSemaphore(sem_read) == -1) break;

    if (shared_buf->shutdown_flag == 1) break;

    int line_len = shared_buf->line_length;
    if (line_len > 0 && line_len < MAX_LINE_LENGTH) {
      char output_buffer[MAX_LINE_LENGTH];
      int output_len = InvertLine(shared_buf->data, line_len, output_buffer);

      if (output_len > 0) {
        WriteToOutput(output_fd, output_buffer, output_len);
      }
    }

    if (PostSemaphore(ipc.sem_ack) == -1) break;
  }

  CloseOutputFile(output_fd);
  CleanupIpc(&ipc, SHM_SIZE);

  return EXIT_SUCCESS;
}
