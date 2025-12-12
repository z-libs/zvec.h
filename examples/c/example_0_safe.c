
#include <stdio.h>
#define ZERROR_IMPLEMENTATION
#define Z_SHORT_ERR 
#include "zerror.h"
#define ZVEC_SHORT_NAMES
#include "zvec.h"

DEFINE_VEC_TYPE(int, Int)

zres populate_and_print(void)
{
    printf("[*] Starting safe vector operations...\n");

    vec_autofree(Int) v = vec_init(Int);

    check( vec_push_safe(&v, 100) );
    check( vec_push_safe(&v, 200) );
    check( vec_push_safe(&v, 300) );

    printf("[*] Vector size: %zu\n", v.length);

    int val = try_into(zres, vec_pop_safe(&v)); 
    printf("    Popped: %d\n", val);

    int first = try_into(zres, vec_at_safe(&v, 0));
    printf("    Index 0: %d\n", first);

    printf("[*] Attempting out-of-bounds access...\n");
    
    int bad = try_into(zres, vec_at_safe(&v, 99)); 

    printf("    Index 99: %d\n", bad); 

    return zres_ok();
}

int main(void)
{
    return run(populate_and_print());
}
