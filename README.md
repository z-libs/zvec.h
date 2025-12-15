# zvec.h

`zvec.h` provides dynamic arrays (vectors) for C projects. Unlike typical C vector implementations that rely on `void*` casting and lose type safety, `zvec.h` uses C11 `_Generic` selection and X-Macros to generate fully typed, type-safe implementations for your specific data structures.

It also includes a robust **C++11 wrapper**, allowing you to use it as a lightweight, drop-in vector class (`z_vec::vector`) in mixed codebases while sharing the same underlying C implementation.

It is part of the [zdk](https://github.com/z-libs/zdk) suite.

## Features

* **Type Safety**: Compiler errors if you try to push a `float` into a `vec_int`.
* **Native Performance**: Data is stored in contiguous arrays of the actual type (no boxing or pointer indirection).
* **C++ Support**: Includes a full C++ class wrapper with RAII, iterators, and `std::vector`-like API.
* **Zero Boilerplate**: Use the **Z-Scanner** tool to automatically generate type registrations.
* **Header Only**: No linking required.
* **Memory Agnostic**: Supports custom allocators (Arenas, Pools, Debuggers).
* **Zero Dependencies**: Only standard C headers used.

## Installation

### Manual

`zvec.h` works best when you use the provided scanner script to manage type registrations, though it can be used manually.

1.  Copy `zvec.h` (and `zcommon.h` if separated) to your project's include folder.
2.  Add the `z-core` tools (optional but recommended):
   
```bash
git submodule add https://github.com/z-libs/z-core.git z-core
```

### Clib

If you use the clib package manager, run:

```bash
clib install z-libs/zdk
```

### ZDK (Recommended)

If you use the Zen Development Kit, it is included automatically by including `<zdk/zvec.h>` (or `<zdk/zworld.h>`).

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
    zvec_Int nums = zvec_init(Int);
    zvec_push(&nums, 42);

    // Initialize Struct Vector.
    zvec_Point path = zvec_init(Point);
    zvec_push(&path, ((Point){1.0f, 2.0f}));

    // Access elements safely (returns T*).
    printf("First number: %d\n", *zvec_at(&nums, 0));
    
    // Cleanup.
    zvec_free(&nums);
    zvec_free(&path);
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

#define REGISTER_ZVEC_TYPES(X) \
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
| `zvec_init(Type)` | Returns an empty vector structure initialized to zero. |
| `zvec_init_with_cap(Type, n)` | Returns a vector with initial memory reserved for `n` elements. |
| `zvec_from(Type, ...)` | Returns a vector populated with the provided elements (for example, `zvec_from(int, 1, 2, 3)`). |
| `zvec_free(v)` | Frees the internal memory array and zeroes the vector structure. |
| `zvec_clear(v)` | Sets the length to 0 but keeps the allocated memory capacity. |
| `zvec_reserve(v, n)` | Ensures the vector has capacity for at least `n` elements total. |
| `zvec_shrink_to_fit(v)` | Reallocates the internal memory to match exactly the current length. |

**Data Access**

| Macro | Description |
| :--- | :--- |
| `zvec_at(v, index)` | Returns a **pointer** to the element at `index`, or `NULL` if out of bounds. |
| `zvec_last(v)` | Returns a **pointer** to the last element in the vector. |
| `zvec_data(v)` | Returns the raw underlying array pointer (`T*`). |
| `zvec_is_empty(v)` | Returns `1` (true) if the vector length is 0, otherwise `0`. |

**Modification**

| Macro | Description |
| :--- | :--- |
| `zvec_push(v, value)` | Appends value to the end. Copies data twice (stack -> heap). Best for simple types. |
| `zvec_push_slot(v, value)` | Reserves space at the end and returns a **pointer** to it. Zero overhead. Best for large structs. |
| `zvec_pop(v)` | Removes the last element. Decrements length. |
| `zvec_pop_get(v)` | Removes the last element and **returns** its value. |
| `zvec_extend(v, arr, count)` | Appends `count` elements from the raw array `arr` to the end of the vector. |
| `zvec_remove(v, index)` | Removes the element at `index`, shifting all subsequent elements left (preserves order). |
| `zvec_swap_remove(v, index)` | Removes the element at `index` by swapping it with the last element (O(1), order not preserved). |
| `zvec_reverse(v)` | Reverses the elements of the vector in-place. |

**Algorithms & Iteration**

| Macro | Description |
| :--- | :--- |
| `zvec_foreach(v, it)` | Standard traversal. **GCC/Clang**: Auto-declares `it`. **Std C**: `it` must be declared before loop. |
| `zvec_foreach_decl(Name, v, it)` | **Portable C99**. Iterates and declares `it` as a pointer inside the loop. Requires type Name. |
| `zvec_sort(v, cmp)` | Sorts the vector in-place using standard `qsort`. `cmp` is a function pointer: `int (*)(const T*, const T*)`. |
| `zvec_bsearch(v, key, cmp)` | Performs a binary search. Returns a pointer to the found element or `NULL`. `key` is `const T*`. |
| `zvec_lower_bound(v, key, cmp)`| Returns a pointer to the first element that does not compare less than `key`. Returns `NULL` if all elements are smaller. |

**Extensions (Experimental)**

If you are using a compiler that supports `__attribute__((cleanup))` (like GCC or Clang), you can use the **Auto-Cleanup** extension to automatically free vectors when they go out of scope.

| Macro | Description |
| :--- | :--- |
| `zvec_autofree(Type)` | Declares a vector that automatically calls `zvec_free` when the variable leaves scope (RAII style). |

**Example:**
```c
void process_data()
{
    // 'nums' will be automatically freed when this function returns.
    zvec_autofree(int) nums = zvec_init(int);
    zvec_push(&nums, 100);
}
```

> **Disable Extensions:** To force standard compliance and disable these extensions, define `Z_NO_EXTENSIONS` before including the library.

### Safe API (Error Handling)

If your project includes **[`zerror.h`](https://github.com/z-libs/zerror.h)**, `zvec.h` automatically exposes a "Safe Mode" API.

Unlike the standard API (which asserts or returns `NULL` on failure), the Safe API returns **Result Objects** (`zres` or `Res_Type`). These results contain full stack traces and debug information if an error occurs (for example, Out of Memory, Index Out of Bounds).

> **Note:** To use the short macros (`try`, `check`, `unwrap`), you must define `#define Z_SHORT_ERR` before including the headers.

**Safe Macros**

| Macro | Returns | Description |
| :--- | :--- | :--- |
| `zvec_push_safe(v, val)` | `zres` | Appends a value. Returns an error if memory allocation fails (OOM). |
| `zvec_reserve_safe(v, n)` | `zres` | Reserves capacity. Returns an error if memory allocation fails. |
| `zvec_at_safe(v, index)` | `Res_Type` | Returns the value at `index`. Returns an error if the index is out of bounds. |
| `zvec_pop_safe(v)` | `Res_Type` | Removes and returns the last element. Returns an error if the vector is empty. |
| `zvec_last_safe(v)` | `Res_Type` | Returns the last element (without removing). Returns an error if empty. |

**Example Usage**

The Safe API is designed to work with `zerror`'s flow control macros.

```c
#define ZERROR_IMPLEMENTATION
#define ZERROR_SHORT_NAMES
#include "zvec.h"
#include "zerror.h"

DEFINE_VEC_TYPE(int, Int)

zres process_data() 
{
    zvec_autofree(Int) nums = zvec_init(Int);

    // -> Check for OOM on push.
    // We use check_ctx to add context to the error if it fails.
    check_ctx(zvec_push_safe(&nums, 100), "Failed to push first item");
    check_ctx(zvec_push_safe(&nums, 200), "Failed to push second item");

    // -> Safe Access (Bounds Checking).
    // We use try_into() because vec_at_safe returns 'Res_Int', 
    // but this function must return 'zres' on failure.
    int val = try_into(zres, zvec_at_safe(&nums, 1));
    
    printf("Value is: %d\n", val);

    // -> Panic on failure (Crash with stack trace).
    // Useful if you are 100% sure the index exists.
    int must_exist = unwrap(zvec_at_safe(&nums, 0));

    return zres_ok();
}

int main(void) 
{
    // If process_data fails, it prints a full error log.
    run(process_data());
    return 0;
}
```

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
#define ZVEC_CALLOC(n, sz)  arena_alloc_zero(frame_arena, (n) * (sz))
#define ZVEC_REALLOC(p, sz) arena_resize(frame_arena, p, sz)
#define ZVEC_FREE(p)        /* no-op for linear arena */
// (ZVEC_MALLOC is strictly unused by zvec internally, but good to define for consistency).

#include "zvec.h"
#include "zlist.h" // zlist will still use standard malloc!
```

## Short Names (Opt-In)

If you prefer a cleaner API and don't have naming conflicts, define `ZVEC_SHORT_NAMES` before including the header. This allows you to use aliases like `vec_push` and `vec_free` instead of their prefixed counterparts.


## Notes

### Why do I need to provide a "Short Name"?

In `REGISTER_TYPES(X)`, you must provide two arguments: the **Actual Type** and a **Short Name**.

```c
//      Actual Type   Short Name
X(unsigned long,        ulong)
```

The reason is that C macros cannot handle spaces when generating names. The library tries to create functions by gluing `zvec_` + `Name`.

If you used `unsigned long` as the name, the macro would try to generate `vec_unsigned long`, which is a syntax error (variable names cannot contain spaces). But, by passing `ulong`, it correctly generates `vec_ulong`.
