#include "thread.h"
#include "triangle_finder.h"

#define MAX_THREADS 64

Triangle best_triangle = {.area = -1.0};
pthread_mutex_t mutex;

void get_combination_by_index(int num_points, int combo_i, int* i, int* j,
                              int* k) {
  int count = 0;
  for (int ii = 0; ii < num_points; ii++) {
    for (int jj = ii + 1; jj < num_points; jj++) {
      for (int kk = jj + 1; kk < num_points; kk++) {
        if (count == combo_i) {
          *i = ii;
          *j = jj;
          *k = kk;
          return;
        }
        count++;
      }
    }
  }
}

void* worker_thread(void* arg) {
  ThreadArgs* args = (ThreadArgs*)arg;
  Triangle local_best;
  local_best.area = -1.0;

  int n = args->num_points;
  Point3D* points = args->points;
  long long count = 0;

  for (int combo_i = args->start_i; combo_i < args->end_i; combo_i++) {
    int i;
    int j;
    int k;
    get_combination_by_index(args->num_points, combo_i, &i, &j, &k);
    double area = CalculateTriangleArea(points[i], points[j], points[k]);

    if (area > local_best.area) {
      local_best.area = area;
      local_best.vertices[0] = points[i];
      local_best.vertices[1] = points[j];
      local_best.vertices[2] = points[k];
    }
    count++;
  }

  LockMutex(&mutex);
  if (local_best.area > best_triangle.area) {
    best_triangle = local_best;
    printf("Поток %d: новый максимум %.6f\n", args->thread_id, local_best.area);
  }
  UnlockMutex(&mutex);

  printf("Поток %d завершен (обработано %lld)\n", args->thread_id, count);
  return NULL;
}

int main(int argc, char* argv[]) {
  Point3D* points;
  int num_points;

  if (argc == 3 && strcmp(argv[2], "-") != 0) {
    ReadPointsFromFile(argv[2], &points, &num_points);
  } else {
    printf("usage: %s <threads> [file|num_points]\n", argv[0]);
    return 1;
  }

  int num_threads = atoi(argv[1]);
  if (num_threads <= 0 || num_threads > MAX_THREADS) {
    num_threads = MAX_THREADS;
  }

  long long total_combinations =
      (long long)num_points * (num_points - 1) * (num_points - 2) / 6;
  printf("Всего комбинаций: %lld\n\n", total_combinations);

  if (CreateMutex(&mutex) != 0) {
    perror("error while CreateMutex");
    free(points);
    return EXIT_FAILURE;
  }

  Thread* threads = malloc(num_threads * sizeof(Thread));
  ThreadArgs* args = malloc(num_threads * sizeof(ThreadArgs));
  if (!threads || !args) {
    perror("error while malloc threads/args");
    free(points);
    DestroyMutex(&mutex);
    return EXIT_FAILURE;
  }

  int base_combo = total_combinations / num_threads;
  int extra_combo = total_combinations % num_threads;
  int start = 0;
  for (int i = 0; i < num_threads; i++) {
    args[i].thread_id = i;
    args[i].points = points;
    args[i].num_points = num_points;

    int count;
    if (i < extra_combo) {
      count = base_combo + 1;
    } else {
      count = base_combo;
    }
    args[i].start_i = start;
    args[i].end_i = start + count;
    start += count;

    ThreadInit(&threads[i], worker_thread);
    if (ThreadRun(&threads[i], &args[i]) != 0) {
      perror("error while Thread_run");
      num_threads = i;
      break;
    }

    printf("Поток %d: комбинации %d - %d (всего %d)\n", i, args[i].start_i,
           args[i].end_i, args[i].end_i - args[i].start_i);
  }

  for (int i = 0; i < num_threads; i++) {
    if (ThreadJoin(&threads[i]) != 0) {
      perror("error while Thread_join");
    }
  }

  printf("РЕЗУЛЬТАТЫ\n");
  if (best_triangle.area > 0) {
    printf("Максимальная площадь: %.6f\n", best_triangle.area);
    printf("Вершины:\n");
    for (int i = 0; i < 3; i++) {
      printf("P%d: (%.6f, %.6f, %.6f)\n", i + 1, best_triangle.vertices[i].x,
             best_triangle.vertices[i].y, best_triangle.vertices[i].z);
    }
  }

  free(threads);
  free(args);
  free(points);
  DestroyMutex(&mutex);

  return 0;
}