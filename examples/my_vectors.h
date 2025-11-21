
#ifndef MY_VECTORS_H
#define MY_VECTORS_H

#include "zvec.h"

typedef struct {
    float x, y;
} Point;

#define REGISTER_TYPES(X) \
    X(int)                \
    X(float)              \
    X(Point)

REGISTER_TYPES(DEFINE_VEC_TYPE)

#endif
