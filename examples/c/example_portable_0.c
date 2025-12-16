
#include <stdio.h>

typedef struct 
{
    float x, y;
} Point;

#define ZVEC_SHORT_NAMES
#include "zvec.h"

DEFINE_VEC_TYPE(int, Int)
DEFINE_VEC_TYPE(Point, Point)

int main(void) 
{
    vec(Int) nums = vec_init(Int);

    vec_push(&nums, 10);
    vec_push(&nums, 20);
    vec_push(&nums, 30);

    printf("Integers: ");

    int *ptr;
    vec_foreach(&nums, ptr) 
    {
        printf("%d ", *ptr);
    }
    printf("\n");

    vec(Point) points = vec_init(Point);
    vec_push(&points, ((Point){1.5f, 2.5f}));
    
    Point *p0 = vec_at(&points, 0);
    if(p0) 
    {
        printf("Point 0: {x: %.1f, y: %.1f}\n", p0->x, p0->y);
    }

    vec_free(&nums);
    vec_free(&points);

    return 0;
}
