#include "triangle_finder.h"

Point3D CrossProduct(Point3D a, Point3D b) {
  Point3D result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

double VectorMagnitude(Point3D v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

double CalculateTriangleArea(Point3D a, Point3D b, Point3D c) {
  Point3D ab = {b.x - a.x, b.y - a.y, b.z - a.z};
  Point3D ac = {c.x - a.x, c.y - a.y, c.z - a.z};
  Point3D cross = CrossProduct(ab, ac);
  return 0.5 * VectorMagnitude(cross);
}

int ReadPointsFromFile(const char* filename, Point3D** points,
                       int* num_points) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    perror("error while open file");
    return EXIT_FAILURE;
  }

  unsigned char bom[3];
  size_t n = fread(bom, 1, 3, file);
  if (n == 3) {
    if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
      fseek(file, 0, SEEK_SET);
    }
  } else {
    fseek(file, 0, SEEK_SET);
  }

  *num_points = 0;
  double x, y, z;
  while (fscanf(file, "%lf %lf %lf", &x, &y, &z) == 3) {
    (*num_points)++;
  }

  if (*num_points < 3) {
    perror("error: not enough points");
    fclose(file);
    return EXIT_FAILURE;
  }

  *points = malloc(*num_points * sizeof(Point3D));
  if (!*points) {
    perror("error while malloc memmory");
    fclose(file);
    return EXIT_FAILURE;
  }

  rewind(file);
  for (int i = 0; i < *num_points; i++) {
    fscanf(file, "%lf %lf %lf", &(*points)[i].x, &(*points)[i].y,
           &(*points)[i].z);
  }

  fclose(file);
  printf("Загружено %d точек\n", *num_points);
  return 0;
}