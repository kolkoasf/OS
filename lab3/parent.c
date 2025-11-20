#include <stdio.h>
#include <string.h>

#include "helpers.h"

volatile sig_atomic_t child1_ready = 0;
volatile sig_atomic_t child2_ready = 0;

void ChildReadyHandler(int sig) {
  if (sig == SIGUSR1) {
    child1_ready = 1;
  } else if (sig == SIGUSR2) {
    child2_ready = 1;
  }
}

static void WaitForChildrenReady(void) {
  while (!child1_ready || !child2_ready) {
    pause();
  }
}

int main(int argc, char* argv[], char* envp[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <file_odd> <file_even>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (RegisterSignalHandler(SIGUSR1, ChildReadyHandler) == -1 ||
      RegisterSignalHandler(SIGUSR2, ChildReadyHandler) == -1) {
    fprintf(stderr, "[Parent] Ошибка регистрации обработчиков сигналов\n");
    return EXIT_FAILURE;
  }

  ipc_handles_t h1 =
      InitIpcHandles(1, "/tmp/child1_input.mmap", "/sem_child1_data_ready",
                     "/sem_child1_processed");
  ipc_handles_t h2 =
      InitIpcHandles(2, "/tmp/child2_input.mmap", "/sem_child2_data_ready",
                     "/sem_child2_processed");

  if (h1.data_ready == NULL || h2.data_ready == NULL) {
    fprintf(stderr, "[Parent] Ошибка инициализации IPC\n");
    CleanupChildIpc(&h1);
    CleanupChildIpc(&h2);
    return EXIT_FAILURE;
  }

  char* args1[] = {"child", "1", "/tmp/child1_input.mmap", argv[1], NULL};
  process_id_t pid1 = CreateProc("./child", args1, envp);

  if (pid1 == -1) {
    fprintf(stderr, "[Parent] Ошибка при создании child1\n");
    CleanupChildIpc(&h2);
    CleanupChildIpc(&h1);
    return EXIT_FAILURE;
  }

  char* args2[] = {"child", "2", "/tmp/child2_input.mmap", argv[2], NULL};
  process_id_t pid2 = CreateProc("./child", args2, envp);

  if (pid2 == -1) {
    fprintf(stderr, "[Parent] Ошибка при создании child2\n");
    CleanupChildIpc(&h2);
    CleanupChildIpc(&h1);
    return EXIT_FAILURE;
  }

  WaitForChildrenReady();

  shared_buffer_t* buf1 = (shared_buffer_t*)h1.input_mmap.addr;
  shared_buffer_t* buf2 = (shared_buffer_t*)h2.input_mmap.addr;

  printf("Введите строки (пустая строка для завершения):\n");
  char line[MAX_LINE_LENGTH];
  int line_number = 0;

  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (line[0] == '\n') {
      break;
    }

    line_number++;
    size_t len = strlen(line);

    if (line_number % 2 == 1) {
      SendDataToChild(buf1, line, len, h1.data_ready, h1.processed);
    } else {
      SendDataToChild(buf2, line, len, h2.data_ready, h2.processed);
    }
  }

  SignalChildShutdown(buf1, h1.data_ready);
  SignalChildShutdown(buf2, h2.data_ready);

  int status1, status2;
  WaitForChildren(pid1, pid2, &status1, &status2);

  CleanupChildIpc(&h1);
  CleanupChildIpc(&h2);

  return EXIT_SUCCESS;
}