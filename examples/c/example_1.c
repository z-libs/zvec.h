
#include <stdio.h>
#include "my_vectors.h"

int main(void)
{
    printf("*Integer example*\n");
    vec_int nums = vec_init(int);
    
    vec_push(&nums, 100);
    vec_push(&nums, 200);
    vec_push(&nums, 300);
    vec_push(&nums, 400);

    printf("Before remove: ");
    int *n;
    vec_foreach(&nums, n)
    {
        printf("%d ", *n);
    }
    printf("\n");

    printf("Removing index 1 (ordered)...\n");
    vec_remove(&nums, 1); 

    printf("After remove:  ");
    vec_foreach(&nums, n)
    {
        printf("%d ", *n);
    }
    printf("\n\n");

    printf("*Point example*\n");
    vec_Point points = vec_init(Point);

    vec_push(&points, ((Point){1.0f, 1.0f}));
    vec_push(&points, ((Point){2.0f, 2.0f}));
    vec_push(&points, ((Point){3.0f, 3.0f}));
    vec_push(&points, ((Point){4.0f, 4.0f}));

    Point *p;
    int i = 0;
    vec_foreach(&points, p) 
    {
        printf("[%d] {x:%.1f, y:%.1f}\n", i++, p->x, p->y);
    }

    printf("Swap removing index 1...\n");
    vec_swap_remove(&points, 1);

    i = 0;
    vec_foreach(&points, p) 
    {
        printf("[%d] {x:%.1f, y:%.1f}\n", i++, p->x, p->y);
    }

    vec_free(&nums);
    vec_free(&points);

    return 0;
}
