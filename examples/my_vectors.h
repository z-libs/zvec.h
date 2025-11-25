
#ifndef MY_VECTORS_H
#define MY_VECTORS_H

typedef struct {
    float x, y;
} Point;

#define REGISTER_TYPES(X)     \
    X(int, int)               \
    X(Point, Point)

#include "zvec.h"

#endif
