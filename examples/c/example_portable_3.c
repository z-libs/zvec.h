
#include <stdio.h>

#define REGISTER_ZVEC_TYPES(X)  \
    X(int, Int)

#define ZVEC_SHORT_NAMES
#include "zvec.h"

int compare_ints(const int *a, const int *b) 
{
    return (*a > *b) - (*a < *b);
}

int main(void) 
{
    vec(Int) nums = vec_init(Int);
    
    vec_push(&nums, 100);
    vec_push(&nums, 50);
    
    int bulk_data[] = { 10, 75, 25, 5 };
    printf("[Extend] Adding {10, 75, 25, 5} to vector...\n");
    
    vec_extend(&nums, bulk_data, 4);

    printf("Current Vector: ");
    int *n;
    vec_foreach(&nums, n) printf("%d ", *n);
    printf("\n\n");

    printf("[Reverse] Reversing vector in-place...\n");
    vec_reverse(&nums);
    
    printf("Reversed:       ");
    vec_foreach(&nums, n) printf("%d ", *n);
    printf("\n\n");

    printf("[Sort] Sorting vector...\n");
    vec_sort(&nums, compare_ints);

    printf("Sorted:         ");
    vec_foreach(&nums, n) printf("%d ", *n);
    printf("\n\n");

    int key_found = 75;
    int key_missing = 99;

    printf("[Binary Search] Looking for values...\n");
    
    int *res1 = vec_bsearch(&nums, &key_found, compare_ints);
    if (res1) 
    {
        long index = res1 - vec_data(&nums); 
        printf("  Key %d: Found at index %ld\n", key_found, index);
    } 
    else 
    {
        printf("  Key %d: Not found\n", key_found);
    }

    int *res2 = vec_bsearch(&nums, &key_missing, compare_ints);
    if (res2)
    {
        printf("  Key %d: Found\n", key_missing);
    } 
    else 
    {
        printf("  Key %d: Not found\n", key_missing);
    }
    printf("\n");

    printf("[Data Access] Accessing raw array directly...\n");
    int *raw_array = vec_data(&nums);
    printf("  raw_array[0] = %d\n", raw_array[0]);
    printf("  raw_array[1] = %d\n", raw_array[1]);

    vec_free(&nums);
    return 0;
}
