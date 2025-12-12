
#define Z_SHORT_ERR
#define ZERROR_IMPLEMENTATION
#include "zvec.h"
#include "zerror.h"

DEFINE_VEC_TYPE(int, Int)

Res_Int get_third_item(vec_Int* v) 
{
    int val = try(vec_at_safe(v, 2)); 
    return Res_Int_ok(val);
}

zres process_data() 
{
    vec_autofree(Int) nums = vec_init(Int);

    check_ctx(vec_push_safe(&nums, 20), "Failed to push 20");
    check_ctx(vec_push_safe(&nums, 30), "Failed to push 30");

    int x = try_into(zres, vec_at_safe(&nums, 0));
    (void)x;

    unwrap(vec_at_safe(&nums, 99)); 

    return zres_ok();
}

int main(void) 
{
    run(process_data());
    return 0;
}
