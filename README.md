# zvec.h

`zvec.h` provides dynamic arrays (vectors) for C projects. Unlike typical C vector implementations that rely on `void*` casting and lose type safety, `zvec.h` uses C11 `_Generic` selection and X-Macros to generate fully typed, type-safe implementations for your specific data structures.

## Features

* **Type Safety**: Compiler errors if you try to push a `float` into a `vec_int`.
* **Native Performance**: Data is stored in contiguous arrays of the actual type (no boxing or pointer indirection).
* **Zero Boilerplate**: Use the **Z-Scanner** tool to automatically generate type registrations.
* **Header Only**: No linking required.
* **Memory Agnostic**: Supports custom allocators (Arenas, Pools, Debuggers).
* **Zero Dependencies**: Only standard C headers used.

## Quick Start (Automated)

The easiest way to use `zvec.h` is with the **Z-Scanner** tool, which scans your code and handles the boilerplate for you.

### 1. Setup

Add `zvec.h` and the `z-core` tools to your project:

```bash
# Copy zvec.h to your root or include folder.
git submodule add https://github.com/z-libs/z-core.git z-core
```

### 2. Write Code

You don't need a separate registry file. Just define the types you need right where you use them (or in your own headers).

```c
#include <stdio.h>
#include "zvec.h"

// Define your struct.
typedef struct { float x, y; } Point;

// Request the vector types you need.
// (These are no-ops for the compiler, but markers for the scanner).
DEFINE_VEC_TYPE(int, Int)
DEFINE_VEC_TYPE(Point, Point)

int main(void)
{
    vec_Int nums = vec_init(Int);
    vec_push(&nums, 42);

    vec_Point path = vec_init(Point);
    vec_push(&path, ((Point){1.0f, 2.0f}));

    printf("First number: %d\n", *vec_at(&nums, 0));
    
    vec_free(&nums);
    vec_free(&path);
    return 0;
}
```

### 3. Build

Run the scanner before compiling. It will create a header that `zvec.h` automatically detects.

```bash
# Scan your source folder (for example, src/ or .) and output to 'z_registry.h'.
python3 z-core/zscanner.py . z_registry.h

# Compile (Include the folder where z_registry.h lives, or just move it).
gcc main.c -I. -o game
```

## Manual Setup

If you cannot use Python or prefer manual control, you can use the **Registry Header** approach.

* Create a file named `my_vectors.h` (or something else).
* Register your types using X-Macros.

```c
#ifndef MY_VECTORS_H
#define MY_VECTORS_H

#define REGISTER_TYPES(X) \
    X(int, Int)           \
    X(float, Float)

// **IT HAS TO BE INCLUDED AFTER, NOT BEFORE**.
#include "zvec.h"

#endif
```

* Include `"my_vectors.h"` instead of `"zvec.h"` in your C files.

## API Reference

`zvec.h` uses C11 `_Generic` to automatically select the correct function implementation based on the vector type you pass.

### Initialization & Management

| Macro | Description |
| :--- | :--- |
| `vec_init(Type)` | Returns an empty vector structure initialized to zero. |
| `vec_init_with_cap(Type, n)` | Returns a vector with initial memory reserved for `n` elements. |
| `vec_from(Type, ...)` | Returns a vector populated with the provided elements (e.g., `vec_from(int, 1, 2, 3)`). |
| `vec_free(v)` | Frees the internal memory array and zeroes the vector structure. |
| `vec_clear(v)` | Sets the length to 0 but keeps the allocated memory capacity. |
| `vec_reserve(v, n)` | Ensures the vector has capacity for at least `n` elements total. |
| `vec_shrink_to_fit(v)` | Reallocates the internal memory to match exactly the current length. |

### Data Access

| Macro | Description |
| :--- | :--- |
| `vec_at(v, index)` | Returns a **pointer** to the element at `index`, or `NULL` if out of bounds. |
| `vec_last(v)` | Returns a **pointer** to the last element in the vector. |
| `vec_data(v)` | Returns the raw underlying array pointer (`T*`). |
| `vec_is_empty(v)` | Returns `1` (true) if the vector length is 0, otherwise `0`. |

### Modification

| Macro | Description |
| :--- | :--- |
| `vec_push(v, value)` | Appends value to the end. Copies data twice (stack -> heap). Best for simple types. |
| `vec_push_slot(v, value)` | Reserves space at the end and returns a **pointer** to it. Zero overhead. Best for large structs. |
| `vec_pop(v)` | Removes the last element. Decrements length. |
| `vec_pop_get(v)` | Removes the last element and **returns** its value. |
| `vec_extend(v, arr, count)` | Appends `count` elements from the raw array `arr` to the end of the vector. |
| `vec_remove(v, index)` | Removes the element at `index`, shifting all subsequent elements left (preserves order). |
| `vec_swap_remove(v, index)` | Removes the element at `index` by swapping it with the last element (O(1), order not preserved). |
| `vec_reverse(v)` | Reverses the elements of the vector in-place. |

### Algorithms & Iteration

| Macro | Description |
| :--- | :--- |
| `vec_foreach(v, iter)` | A loop helper. `iter` must be a pointer variable; it is assigned to each element in the vector sequentially. |
| `vec_sort(v, cmp)` | Sorts the vector in-place using standard `qsort`. `cmp` is a function pointer: `int (*)(const T*, const T*)`. |
| `vec_bsearch(v, key, cmp)` | Performs a binary search. Returns a pointer to the found element or `NULL`. `key` is `const T*`. |
| `vec_lower_bound(v, key, cmp)`| Returns a pointer to the first element that does not compare less than `key`. Returns `NULL` if all elements are smaller. |

## Extensions (Experimental)

If you are using a compiler that supports `__attribute__((cleanup))` (like GCC or Clang), you can use the **Auto-Cleanup** extension to automatically free vectors when they go out of scope.

| Macro | Description |
| :--- | :--- |
| `vec_autofree(Type)` | Declares a vector that automatically calls `vec_free` when the variable leaves scope (RAII style). |

**Example:**
```c
void process_data()
{
    // 'nums' will be automatically freed when this function returns.
    vec_autofree(int) nums = vec_init(int);
    vec_push(&nums, 100);
}
```

> **Disable Extensions:** To force standard compliance and disable these extensions, define `Z_NO_EXTENSIONS` before including the library.

## Memory Management

By default, `zvec.h` uses the standard C library functions (`malloc`, `calloc`, `realloc`, `free`).

However, you can override these to use your own memory subsystem (e.g., **Memory Arenas**, **Pools**, or **Debug Allocators**).

### First Option: Global Override (Recommended)

To use a custom allocator, define the `Z_` macros **inside your registry header**, immediately before including `zvec.h`.

**Example: my_vectors.h**

```c
#ifndef MY_VECTORS_H
#define MY_VECTORS_H

// Define your custom memory macros **HERE**.
#include "my_memory_system.h"

// IMPORTANT: Override all four to prevent mixing allocators.
//            This applies to all the z-libs.
#define Z_MALLOC(sz)      my_custom_alloc(sz)
#define Z_CALLOC(n, sz)   my_custom_calloc(n, sz)
#define Z_REALLOC(p, sz)  my_custom_realloc(p, sz)
#define Z_FREE(p)         my_custom_free(p)


// Then include the library.
#include "zvec.h"

// ... Register types ...


#endif
```

> **Note:** You **must** override **all four macros** (`MALLOC`, `CALLOC`, `REALLOC`, `FREE`) if you override one, to ensure consistency.

### Second Option: Library-Specific Override (Advanced)

If you need different allocators for different containers (e.g., an Arena for Lists but the Heap for Vectors), you can use the library-specific macros. These take priority over the global `Z_` macros.

```c
// Example: Vectors use a Frame Arena, everything else uses standard malloc.
#define Z_VEC_CALLOC(n, sz)  arena_alloc_zero(frame_arena, (n) * (sz))
#define Z_VEC_REALLOC(p, sz) arena_resize(frame_arena, p, sz)
#define Z_VEC_FREE(p)        /* no-op for linear arena */
// (Z_VEC_MALLOC is strictly unused by zvec internally, but good to define for consistency).

#include "zvec.h"
#include "zlist.h" // zlist will still use standard malloc!
```

## Notes

### Why do I need to provide a "Short Name"?

In `REGISTER_TYPES(X)`, you must provide two arguments: the **Actual Type** and a **Short Name**.

```c
//      Actual Type   Short Name
X(unsigned long,        ulong)
```

The reason is that C macros cannot handle spaces when generating names. The library tries to create functions by gluing `vec_` + `Name`.

If you used `unsigned long` as the name, the macro would try to generate `vec_unsigned long`, which is a syntax error (variable names cannot contain spaces). But, by passing `ulong`, it correctly generates `vec_ulong`.
