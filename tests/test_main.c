#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct 
{ 
    float x, y; 
} Vec2;

#define REGISTER_ZVEC_TYPES(X) \
    X(int, Int)                \
    X(Vec2, Vec2)

#include "zvec.h"

#define TEST(name) printf("[TEST] %-35s", name);
#define PASS() printf(" \033[0;32mPASS\033[0m\n")

int cmp_int(const int *a, const int *b) 
{ 
    return *a - *b; 
}
int cmp_vec2_x(const Vec2 *a, const Vec2 *b) 
{
    return (int)(a->x - b->x); 
}


void test_init_management(void) 
{
    TEST("Init, From, Reserve, Shrink, Clear");

    // zvec_init.
    zvec_Int v1 = zvec_init(Int);
    assert(zvec_is_empty(&v1));
    assert(v1.capacity == 0);

    // zvec_init_with_cap.
    zvec_Int v2 = zvec_init_with_cap(Int, 50);
    assert(v2.length == 0);
    assert(v2.capacity == 50); // Exact reservation check.

    // zvec_from (Variadic init).
    zvec_Int v3 = zvec_from(Int, 10, 20, 30);
    assert(v3.length == 3);
    assert(*zvec_at(&v3, 0) == 10);
    assert(*zvec_at(&v3, 2) == 30);

    // zvec_reserve.
    zvec_reserve(&v1, 100);
    assert(v1.capacity >= 100);

    // zvec_clear.
    zvec_clear(&v3);
    assert(v3.length == 0);
    assert(v3.capacity > 0); // Capacity should remain.

    // zvec_shrink_to_fit.
    // Push one item, then shrink. Cap should equal len (1).
    zvec_push(&v3, 99); 
    zvec_shrink_to_fit(&v3);
    assert(v3.capacity == 1);

    zvec_free(&v1);
    zvec_free(&v2);
    zvec_free(&v3);
    PASS();
}

void test_data_access(void) 
{
    TEST("At, Last, Data, Is_Empty");

    zvec_Int v = zvec_init(Int);
    assert(zvec_is_empty(&v) == 1);

    zvec_push(&v, 100);
    zvec_push(&v, 200);

    assert(zvec_is_empty(&v) == 0);
    
    // zvec_at (bounds check logic inside macro returns NULL).
    assert(*zvec_at(&v, 0) == 100);
    assert(zvec_at(&v, 999) == NULL);

    // zvec_last.
    assert(*zvec_last(&v) == 200);

    // zvec_data.
    int* raw = zvec_data(&v);
    assert(raw[0] == 100);
    assert(raw[1] == 200);

    zvec_free(&v);
    PASS();
}

void test_modification(void) 
{
    TEST("Push, Slot, Pop, Extend, Remove");

    zvec_Int v = zvec_init(Int);

    // zvec_push.
    zvec_push(&v, 1);
    zvec_push(&v, 2);
    zvec_push(&v, 3); // [1, 2, 3].

    // zvec_pop.
    zvec_pop(&v); // [1, 2].
    assert(v.length == 2);
    assert(*zvec_last(&v) == 2);

    // zvec_pop_get.
    int val = zvec_pop_get(&v); // Returns 2, list is [1].
    assert(val == 2);
    assert(v.length == 1);

    // zvec_extend.
    int arr[] = {4, 5, 6};
    zvec_extend(&v, arr, 3); // [1, 4, 5, 6].
    assert(v.length == 4);
    assert(*zvec_last(&v) == 6);

    // zvec_remove (Preserves order).
    // Remove index 1 (value 4). Result: [1, 5, 6].
    zvec_remove(&v, 1);
    assert(v.length == 3);
    assert(*zvec_at(&v, 0) == 1);
    assert(*zvec_at(&v, 1) == 5);
    assert(*zvec_at(&v, 2) == 6);

    // zvec_swap_remove (O(1), breaks order).
    // Remove index 0 (value 1). Swaps with last (6). Result: [6, 5].
    zvec_swap_remove(&v, 0);
    assert(v.length == 2);
    assert(*zvec_at(&v, 0) == 6); 
    assert(*zvec_at(&v, 1) == 5);

    // zvec_reverse.
    zvec_reverse(&v); // [5, 6].
    assert(*zvec_at(&v, 0) == 5);

    // zvec_push_slot (Struct optimization).
    zvec_Vec2 points = zvec_init(Vec2);
    Vec2* p = zvec_push_slot(&points); // Reserves and returns ptr.
    p->x = 10.0f; 
    p->y = 20.0f;
    assert(points.length == 1);
    assert(zvec_at(&points, 0)->x == 10.0f);

    zvec_free(&v);
    zvec_free(&points);
    PASS();
}

void test_algorithms(void) 
{
    TEST("Sort, BSearch, LowerBound, Foreach");

    zvec_Int v = zvec_from(Int, 50, 10, 30, 20, 40);

    // zvec_foreach.
    int sum = 0;
    zvec_foreach(&v, it) 
    {
        sum += *it;
    }
    assert(sum == 150);

    // zvec_sort.
    zvec_sort(&v, cmp_int); // [10, 20, 30, 40, 50].
    assert(*zvec_at(&v, 0) == 10);
    assert(*zvec_at(&v, 4) == 50);

    // zvec_bsearch.
    int key = 30;
    int* res = zvec_bsearch(&v, &key, cmp_int);
    assert(res != NULL && *res == 30);
    
    key = 99;
    res = zvec_bsearch(&v, &key, cmp_int);
    assert(res == NULL);

    // zvec_lower_bound.
    // List: [10, 20, 30, 40, 50].
    // LB(25) should be 30 (index 2).
    key = 25;
    res = zvec_lower_bound(&v, &key, cmp_int);
    assert(res != NULL && *res == 30);

    // LB(5) should be 10 (index 0).
    key = 5;
    res = zvec_lower_bound(&v, &key, cmp_int);
    assert(res != NULL && *res == 10);

    // LB(60) should be NULL (end).
    key = 60;
    res = zvec_lower_bound(&v, &key, cmp_int);
    assert(res == NULL);

    zvec_free(&v);
    PASS();
}

// Extension test (GCC/Clang only).
#if defined(__GNUC__) || defined(__clang__)
void test_autofree(void) 
{
    TEST("Auto-Cleanup Extension");
    
    // We create a scope block to verify compilation and execution.
    // But verifying actual memory release requires Valgrind/ASAN.
    {
        zvec_autofree(Int) auto_v = zvec_init(Int);
        zvec_push(&auto_v, 123);
        assert(*zvec_at(&auto_v, 0) == 123);
    } // auto_v is freed here.
    
    PASS();
}
#endif

int main(void) 
{
    printf("=> Running tests (zvec.h, main).\n");
    
    test_init_management();
    test_data_access();
    test_modification();
    test_algorithms();

#if defined(__GNUC__) || defined(__clang__)
    test_autofree();
#endif

    printf("=> All tests passed successfully.\n");
    return 0;
}
