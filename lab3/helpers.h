#include "os.h"

#define MAX_LINE_LENGTH 1024
#define MAPPED_FILE_SIZE (100 * MAX_LINE_LENGTH)

typedef struct {
  volatile int line_length;
  volatile int shutdown_flag;
  char data[MAX_LINE_LENGTH];
} shared_buffer_t;

typedef struct {
  semaphore_t data_ready;
  semaphore_t processed;
  mmap_info_t input_mmap;
  pipe_t input_fd;
  const char *data_ready_name;
  const char *processed_name;
} ipc_handles_t;

typedef struct {
  int id;
  const char *input_mmap_path;
  const char *output_file_path;
} child_config_t;

ipc_handles_t InitIpcHandles(int child_id, const char *input_path,
                             const char *data_ready_name,
                             const char *processed_name);

void CleanupChildIpc(ipc_handles_t *handles);

int SendDataToChild(shared_buffer_t *buf, const char *data, size_t len,
                    semaphore_t sem_ready, semaphore_t sem_processed);

void SignalChildShutdown(shared_buffer_t *buf, semaphore_t sem_ready);

void WaitForChildren(process_id_t pid1, process_id_t pid2, int *status1,
                     int *status2);

int InitChildConfig(int argc, char *argv[], child_config_t *config);

ipc_handles_t SetupChildIpc(const child_config_t *config);

void CleanupChildIpcSafe(ipc_handles_t *handles);

int CreateOutputFile(const child_config_t *config);

int SignalReady(const child_config_t *config);

int WriteToOutput(int fd, const char *data, size_t data_len, int child_id);
