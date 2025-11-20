#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

int main(int argc, char* argv[], char* envp[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
    return EXIT_FAILURE;
  }

  ipc_handles_t ipc = InitParentIpc(SHM_NAME, SHM_SIZE);
  if (ipc.shm_fd == -1) {
    fprintf(stderr, "[Parent] Failed to initialize IPC\n");
    return EXIT_FAILURE;
  }

  char* args1[] = {"child", "1", (char*)SHM_NAME, argv[1], NULL};
  process_id_t pid1 = CreateProc("./child", args1, envp);
  if (pid1 == -1) {
    fprintf(stderr, "[Parent] Failed to create child1\n");
    CleanupIpcFull(&ipc, SHM_NAME, SHM_SIZE);
    return EXIT_FAILURE;
  }

  char* args2[] = {"child", "2", (char*)SHM_NAME, argv[2], NULL};
  process_id_t pid2 = CreateProc("./child", args2, envp);
  if (pid2 == -1) {
    fprintf(stderr, "[Parent] Failed to create child2\n");
    CleanupIpcFull(&ipc, SHM_NAME, SHM_SIZE);
    return EXIT_FAILURE;
  }

  sleep(1);

  shared_buffer_t* shared_buf = (shared_buffer_t*)ipc.shm_addr;
  fprintf(stdout, "Enter lines (empty line or Ctrl+D to quit):\n");
  fflush(stdout);

  char line[MAX_LINE_LENGTH];
  int line_number = 0;

  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (line[0] == '\n') break;

    line_number++;
    size_t len = strlen(line);
    int target_child = (line_number % 2 == 1) ? 1 : 2;

    if (SendDataToChild(shared_buf, line, len, target_child, &ipc) == -1) {
      fprintf(stderr, "[Parent] Failed to send data\n");
      break;
    }
  }

  SignalChildShutdown(shared_buf, &ipc);

  int status1, status2;
  WaitForChildren(pid1, pid2, &status1, &status2);

  CleanupIpcFull(&ipc, SHM_NAME, SHM_SIZE);

  return EXIT_SUCCESS;
}
