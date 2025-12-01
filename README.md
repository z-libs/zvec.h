# zvec.h

`zvec.h` provides dynamic arrays (vectors) for C projects. Unlike typical C vector implementations that rely on `void*` casting and lose type safety, `zvec.h` uses C11 `_Generic` selection and X-Macros to generate fully typed, type-safe implementations for your specific data structures.

It also includes a robust **C++11 wrapper**, allowing you to use it as a lightweight, drop-in vector class (`z_vec::vector`) in mixed codebases while sharing the same underlying C implementation.

## Features

* **Type Safety**: Compiler errors if you try to push a `float` into a `vec_int`.
* **Native Performance**: Data is stored in contiguous arrays of the actual type (no boxing or pointer indirection).
* **C++ Support**: Includes a full C++ class wrapper with RAII, iterators, and `std::vector`-like API.
* **Zero Boilerplate**: Use the **Z-Scanner** tool to automatically generate type registrations.
* **Header Only**: No linking required.
* **Memory Agnostic**: Supports custom allocators (Arenas, Pools, Debuggers).
* **Zero Dependencies**: Only standard C headers used.

## Installation

`zvec.h` works best when you use the provided scanner script to manage type registrations, though it can be used manually.

1.  Copy `zvec.h` (and `zcommon.h` if separated) to your project's include folder.
2.  Add the `z-core` tools (optional but recommended):
    ```bash
    git submodule add [https://github.com/z-libs/z-core.git](https://github.com/z-libs/z-core.git) z-core
    ```

## Usage: C

For C projects, you define the vector types you need using a macro that the scanner detects.

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
    // Initialize (Standard C style).
    vec_Int nums = vec_init(Int);
    vec_push(&nums, 42);

    // Initialize Struct Vector.
    vec_Point path = vec_init(Point);
    vec_push(&path, ((Point){1.0f, 2.0f}));

    // Access elements safely (returns T*).
    printf("First number: %d\n", *vec_at(&nums, 0));
    
    // Cleanup.
    vec_free(&nums);
    vec_free(&path);
    return 0;
}
```

## Usage: C++

The library detects C++ compilers automatically. The C++ wrapper lives in the **`z_vec`** namespace and wraps the underlying C implementation transparently.

```cpp
#include <iostream>
#include "zvec.h"

struct Point { float x, y; };

// Request types (scanner sees this even in .cpp files).
DEFINE_VEC_TYPE(int, Int)
DEFINE_VEC_TYPE(Point, Point)

int main()
{
    // RAII handles memory automatically.
    z_vec::vector<int> nums = {1, 2, 3};

    // Standard push_back API.
    nums.push_back(42);

    // Range-based for loops supported.
    for(int n : nums) {
        std::cout << n << " ";
    }

    // Bounds checking access.
    try {
        nums.at(99) = 10;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
```

## Compilation Guide

Since `zvec.h` relies on code generation for its type safety, here you have the guide if you use the scanner.

**[Read the Compilation Guide](examples/README.md)** for detailed instructions on how to use `zscanner.py`.

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

## API Reference (C)

`zvec.h` uses C11 `_Generic` to automatically select the correct function implementation based on the vector type you pass.

**Initialization & Management**

| Macro | Description |
| :--- | :--- |
| `vec_init(Type)` | Returns an empty vector structure initialized to zero. |
| `vec_init_with_cap(Type, n)` | Returns a vector with initial memory reserved for `n` elements. |
| `vec_from(Type, ...)` | Returns a vector populated with the provided elements (e.g., `vec_from(int, 1, 2, 3)`). |
| `vec_free(v)` | Frees the internal memory array and zeroes the vector structure. |
| `vec_clear(v)` | Sets the length to 0 but keeps the allocated memory capacity. |
| `vec_reserve(v, n)` | Ensures the vector has capacity for at least `n` elements total. |
| `vec_shrink_to_fit(v)` | Reallocates the internal memory to match exactly the current length. |

**Data Access**

| Macro | Description |
| :--- | :--- |
| `vec_at(v, index)` | Returns a **pointer** to the element at `index`, or `NULL` if out of bounds. |
| `vec_last(v)` | Returns a **pointer** to the last element in the vector. |
| `vec_data(v)` | Returns the raw underlying array pointer (`T*`). |
| `vec_is_empty(v)` | Returns `1` (true) if the vector length is 0, otherwise `0`. |

**Modification**

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

**Algorithms & Iteration**

| Macro | Description |
| :--- | :--- |
| `vec_foreach(v, iter)` | A loop helper. `iter` must be a pointer variable; it is assigned to each element in the vector sequentially. |
| `vec_sort(v, cmp)` | Sorts the vector in-place using standard `qsort`. `cmp` is a function pointer: `int (*)(const T*, const T*)`. |
| `vec_bsearch(v, key, cmp)` | Performs a binary search. Returns a pointer to the found element or `NULL`. `key` is `const T*`. |
| `vec_lower_bound(v, key, cmp)`| Returns a pointer to the first element that does not compare less than `key`. Returns `NULL` if all elements are smaller. |

**Extensions (Experimental)**

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

## API Reference (C++)

The C++ wrapper lives in the **`z_vec`** namespace. It strictly adheres to RAII principles and delegates all logic to the underlying C implementation.

### `class z_vec::vector<T>`

**Constructors & Management**

| Method | Description |
| :--- | :--- |
| `vector()` | Default constructor (empty). |
| `vector(size_t cap)` | Constructs with initial capacity reserved. |
| `vector({1, 2, ...})` | Constructs from an initializer list. |
| `~vector()` | Destructor. Automatically calls `vec_free`. |
| `operator=` | Copy and Move assignment operators. |

**Access & Iterators**

| Method | Description |
| :--- | :--- |
| `data()` | Returns `T*` (mutable) or `const T*`. |
| `size()` | Returns current number of elements. |
| `capacity()` | Returns current allocated capacity. |
| `empty()` | Returns `true` if size is 0. |
| `operator[]` | Unchecked access to element at index. |
| `at(index)` | Bounds-checked access. Throws `std::out_of_range`. |
| `front()`, `back()` | Access first/last element. |
| `begin()`, `end()` | Standard iterators (pointers) compatible with STL algorithms. |

**Modification**

| Method | Description |
| :--- | :--- |
| `push_back(val)` | Appends value to the end. |
| `pop_back()` | Removes the last element. |
| `reserve(n)` | Reserves capacity for at least `n` items. |
| `clear()` | Sets size to 0 (capacity remains). |
| `shrink_to_fit()` | Reduces capacity to match size. |
| `reverse()` | Reverses elements in-place. |
| `remove(index)` | Removes element at index (O(N) shift). |
| `swap_remove(index)` | Removes element at index by swapping with last (O(1)). |

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
