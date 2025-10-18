#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#else
#include <pthread.h>
#include <unistd.h>
#endif

typedef struct {
  double x;
  double y;
  double z;
} Point3D;

typedef struct {
  Point3D vertices[3];
  double area;
} Triangle;

typedef struct {
  int thread_id;
  Point3D* points;
  int num_points;
  int start_i;
  int end_i;
} ThreadArgs;

Point3D CrossProduct(Point3D a, Point3D b);
double VectorMagnitude(Point3D v);
double CalculateTriangleArea(Point3D a, Point3D b, Point3D c);

int ReadPointsFromFile(const char* filename, Point3D** points, int* num_points);