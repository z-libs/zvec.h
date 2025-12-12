
#ifndef MY_VECTORS_H
#define MY_VECTORS_H

typedef struct 
{
    float x, y;
} Point;

#define REGISTER_ZVEC_TYPES(X)  \
    X(int, int)                 \
    X(Point, Point)

#define ZVEC_SHORT_NAMES
#include "zvec.h"

#endif
