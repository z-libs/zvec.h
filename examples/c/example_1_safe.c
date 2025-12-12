
typedef struct 
{ 
    int id; 
    float temperature; 
} SensorData;

#include <stdio.h>
#define ZERROR_IMPLEMENTATION
#define Z_SHORT_ERR
#include "zerror.h"
#define ZVEC_SHORT_NAMES
#include "zvec.h"

DEFINE_VEC_TYPE(SensorData, Data)

ResInt process_sensors(void)
{
    printf("[*] Initializing Sensor Data...\n");

    vec_autofree(Data) sensors = vec_init(Data);

    check_into( ResInt, vec_push_safe(&sensors, ((SensorData){101, 24.5f})) );
    check_into( ResInt, vec_push_safe(&sensors, ((SensorData){102, 25.1f})) );
    check_into( ResInt, vec_push_safe(&sensors, ((SensorData){103, 23.8f})) );

    SensorData last = try_into( ResInt, vec_last_safe(&sensors) );
    
    printf("    Last Reading: ID %d -> %.1f C\n", last.id, last.temperature);

    printf("[*] Processing items...\n");
    int count = 0;
    while (!vec_is_empty(&sensors))
    {
        SensorData item = try_into( ResInt, vec_pop_safe(&sensors) );
        printf("    Processed ID: %d\n", item.id);
        count++;
    }

    printf("[*] Attempting to pop from empty vector...\n");
    
    /* Deliberate error... this will return ResInt_err(...) up the stack. */
    SensorData ghost = try_into( ResInt, vec_pop_safe(&sensors) );

    printf("(!) This line is never reached! Got: %d\n", ghost.id);
    
    return ResInt_ok(count);
}

zres app_main(void)
{
    int count = try_into(zres, process_sensors());

    printf("Success! Processed %d items.\n", count);
    
    return zres_ok();
}

int main(void)
{
    return run(app_main());
}
