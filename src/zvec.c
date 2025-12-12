
#ifndef ZVEC_H
#define ZVEC_H

#include "zcommon.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h> 

#if defined(__has_include) && __has_include("zerror.h")
    #include "zerror.h"
    #define Z_HAS_ZERROR 1
#elif defined(ZERROR_H)
    #define Z_HAS_ZERROR 1
#else
    #define Z_HAS_ZERROR 0
#endif

/* C++ interop preamble. */
#ifdef __cplusplus
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <algorithm>

namespace z_vec 
{
    /* Traits struct. */
    // The traits struct connects the C++ wrapper to the generated C functions.
    template <typename T>
    struct traits 
    {
        static_assert(sizeof(T) == 0, "No zvec implementation registered for this type (via DEFINE_VEC_TYPE).");
    };

    /* Vector implementation */
    template <typename T>
    class vector 
    {
        using Traits = traits<T>;
        using CType = typename Traits::c_type;

        CType inner;

    public:
        using value_type      = T;
        using size_type       = size_t;
        using difference_type = std::ptrdiff_t;
        using pointer         = T*;
        using const_pointer   = const T*;
        using iterator        = T*;
        using const_iterator  = const T*;

        /* Constructors and destructor (RAII). */

        // Default constructor.
        vector() { Traits::init(inner); }
        
        // Constructs with initial capacity.
        explicit vector(size_t capacity) { Traits::init_cap(inner, capacity); }

        // Constructs from an initializer list.
        vector(std::initializer_list<T> init) 
        {
            Traits::init_cap(inner, init.size());
            Traits::extend(inner, init.begin(), init.size());
        }

        // Copy constructor (deep copy).
        vector(const vector &other) 
        {
            Traits::init_cap(inner, other.size());
            Traits::extend(inner, other.data(), other.size());
        }

        // Move constructor (transfer ownership).
        vector(vector &&other) noexcept 
        {
            inner = other.inner;
            Traits::init(other.inner);
        }

        // Destructor (frees memory).
        ~vector() { Traits::free(inner); }

        // Copy assignment.
        vector& operator=(const vector &other) 
        {
            if (this != &other) 
            {
                Traits::free(inner);
                Traits::init_cap(inner, other.size());
                Traits::extend(inner, other.data(), other.size());
            }
            return *this;
        }

        // Move assignment.
        vector& operator=(vector &&other) noexcept 
        {
            if (this != &other) 
            {
                Traits::free(inner);
                inner = other.inner;
                Traits::init(other.inner);
            }
            return *this;
        }

        /* Accessors. */

        T* data()             { return inner.data; }
        const T* data() const { return inner.data; }
        
        size_t size() const     { return inner.length; }
        size_t capacity() const { return inner.capacity; }
        bool empty() const      { return inner.length == 0; }

        // Unchecked access.
        T& operator[](size_t idx)             { return inner.data[idx]; }
        const T& operator[](size_t idx) const { return inner.data[idx]; }

        // Checked access (throws std::out_of_range).
        T& at(size_t idx) 
        {
            if (idx >= size()) throw std::out_of_range("vector::at");
            return inner.data[idx];
        }

        const T& at(size_t idx) const 
        {
            if (idx >= size()) throw std::out_of_range("vector::at");
            return inner.data[idx];
        }

        T& front()             { return inner.data[0]; }
        const T& front() const { return inner.data[0]; }

        T& back()             { return inner.data[inner.length - 1]; }
        const T& back() const { return inner.data[inner.length - 1]; }

        /* Modifiers */

        void push_back(const T &val) { Traits::push(inner, val); }
        void pop_back()              { Traits::pop(inner); }
        
        void clear()   { Traits::clear(inner); }
        void reserve(size_t cap) { Traits::reserve(inner, cap); }

        void shrink_to_fit() { Traits::shrink(inner); }
        void reverse()       { Traits::reverse(inner); }

        void remove(size_t idx)      { Traits::remove(inner, idx); }
        void swap_remove(size_t idx) { Traits::swap_remove(inner, idx); }

        /* Iterators */

        iterator begin() { return inner.data; }
        iterator end()   { return inner.data + inner.length; }
        const_iterator begin() const { return inner.data; }
        const_iterator end() const   { return inner.data + inner.length; }
    };
}
extern "C" {
#endif

/* C implementation. */

#ifndef ZVEC_MALLOC
    #define ZVEC_MALLOC(sz)      Z_MALLOC(sz)
#endif

#ifndef ZVEC_CALLOC
    #define ZVEC_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif

#ifndef ZVEC_REALLOC
    #define ZVEC_REALLOC(p, sz)  Z_REALLOC(p, sz)
#endif

#ifndef ZVEC_FREE
    #define ZVEC_FREE(p)         Z_FREE(p)
#endif

/* Safe API generator logic (requires zerror.h). */
#if Z_HAS_ZERROR

    /* * Helper to bridge zvec location to zerror. 
     * We use zerr_create_impl directly to inject the CALLER'S location,
     * not the location inside this header file.
     */
    static inline zerr zvec_err_impl(int code, const char* msg, 
                                     const char* file, int line, const char* func) 
    {
        return zerr_create_impl(code, file, line, func, "%s", msg);
    }

    #define ZVEC_GEN_SAFE_IMPL(T, Name)                                                         \
        /* Define a Result type for this specific element type (for example, Res_Point) */      \
        DEFINE_RESULT(T, Res_##Name)                                                            \
                                                                                                \
        /* Push with OOM check. Returns standard zres (void result). */                         \
        static inline zres zvec_push_safe_##Name(zvec_##Name *v, T value,                       \
                                                const char* f, int l, const char* fn)           \
        {                                                                                       \
            if (v->length >= v->capacity)                                                       \
            {                                                                                   \
                size_t new_cap = Z_GROWTH_FACTOR(v->capacity);                                  \
                T *new_data = (T*)ZVEC_REALLOC(v->data, new_cap * sizeof(T));                   \
                if (!new_data)                                                                  \
                {                                                                               \
                    return zres_err(zvec_err_impl(Z_ERR, "Vector Push OOM", f, l, fn));         \
                }                                                                               \
                v->data = new_data;                                                             \
                v->capacity = new_cap;                                                          \
            }                                                                                   \
            v->data[v->length++] = value;                                                       \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        /* Reserve with OOM check. */                                                           \
        static inline zres zvec_reserve_safe_##Name(zvec_##Name *v, size_t cap,                 \
                                                   const char* f, int l, const char* fn)        \
        {                                                                                       \
            if (cap <= v->capacity) return zres_ok();                                           \
            T *new_data = (T*)ZVEC_REALLOC(v->data, cap * sizeof(T));                           \
            if (!new_data)                                                                      \
            {                                                                                   \
                return zres_err(zvec_err_impl(Z_ERR, "Vector Reserve OOM", f, l, fn));          \
            }                                                                                   \
            v->data = new_data;                                                                 \
            v->capacity = cap;                                                                  \
            return zres_ok();                                                                   \
        }                                                                                       \
                                                                                                \
        /* Pop that returns Result<T>. Fails if empty. */                                       \
        static inline Res_##Name zvec_pop_safe_##Name(zvec_##Name *v,                           \
                                                     const char* f, int l, const char* fn)      \
        {                                                                                       \
            if (v->length == 0)                                                                 \
                return Res_##Name##_err(zvec_err_impl(Z_ERR, "Pop empty vec", f, l, fn));       \
            return Res_##Name##_ok(v->data[--v->length]);                                       \
        }                                                                                       \
                                                                                                \
        /* At (get) that returns Result<T>. Fails if bounds. */                                 \
        static inline Res_##Name zvec_at_safe_##Name(zvec_##Name *v, size_t i,                  \
                                                    const char* f, int l, const char* fn)       \
        {                                                                                       \
            if (i >= v->length)                                                                 \
                return Res_##Name##_err(zvec_err_impl(Z_ERR, "Index out of bounds", f, l, fn)); \
            return Res_##Name##_ok(v->data[i]);                                                 \
        }                                                                                       \
                                                                                                \
        /* Last that returns Result<T>. */                                                      \
        static inline Res_##Name zvec_last_safe_##Name(zvec_##Name *v,                          \
                                                      const char* f, int l, const char* fn)     \
        {                                                                                       \
            if (v->length == 0)                                                                 \
                return Res_##Name##_err(zvec_err_impl(Z_ERR, "Vector is empty", f, l, fn));     \
            return Res_##Name##_ok(v->data[v->length - 1]);                                     \
        }

#else
    #define ZVEC_GEN_SAFE_IMPL(T, Name)
#endif

/* * The generator macro.
 * Expands to a complete implementation of a dynamic array for type T.
 * T    : The element type (e.g., int, float, Point).
 * Name : The suffix for the generated functions (for example, Int -> vec_push_Int).
 * Name must be one word only.
 */
#define ZVEC_GENERATE_IMPL(T, Name)                                                         \
                                                                                            \
typedef T zvec_T_##Name;                                                                    \
                                                                                            \
/* The main vector struct for this type. */                                                 \
typedef struct                                                                              \
{                                                                                           \
    T *data;                                                                                \
    size_t length;                                                                          \
    size_t capacity;                                                                        \
} zvec_##Name;                                                                              \
                                                                                            \
/* Inject Safe API if enabled */                                                            \
ZVEC_GEN_SAFE_IMPL(T, Name)                                                                 \
                                                                                            \
/* Initializes a vector with a specific initial capacity. */                                \
static inline zvec_##Name vec_init_capacity_##Name(size_t cap)                              \
{                                                                                           \
    zvec_##Name v;                                                                          \
    memset(&v, 0, sizeof(zvec_##Name));                                                     \
    if (cap > 0)                                                                            \
    {                                                                                       \
        v.data = (T*)ZVEC_CALLOC(cap, sizeof(T));                                           \
        v.capacity = v.data ? cap : 0;                                                      \
    }                                                                                       \
    return v;                                                                               \
}                                                                                           \
                                                                                            \
/* Creates a new vector by copying elements from a raw array. */                            \
static inline zvec_##Name vec_from_array_##Name(const T *arr, size_t count)                 \
{                                                                                           \
    zvec_##Name v = zvec_init_capacity_##Name(count);                                       \
    if (v.data)                                                                             \
    {                                                                                       \
        memcpy(v.data, arr, count * sizeof(T));                                             \
        v.length = count;                                                                   \
    }                                                                                       \
    return v;                                                                               \
}                                                                                           \
                                                                                            \
/* Ensures the vector has space for at least `new_cap` elements. */                         \
static inline int zvec_reserve_##Name(zvec_##Name *v, size_t new_cap)                       \
{                                                                                           \
    if (new_cap <= v->capacity) return Z_OK;                                                \
    T *new_data = (T*)ZVEC_REALLOC(v->data, new_cap * sizeof(T));                           \
    if (!new_data) return Z_ERR;                                                            \
    v->data = new_data;                                                                     \
    v->capacity = new_cap;                                                                  \
    return Z_OK;                                                                            \
}                                                                                           \
                                                                                            \
/* Returns 1 if the vector is empty, 0 otherwise. */                                        \
static inline int zvec_is_empty_##Name(zvec_##Name *v)                                      \
{                                                                                           \
    return v->length == 0;                                                                  \
}                                                                                           \
                                                                                            \
/* Reserves space for one more item and returns a pointer to the new slot. */               \
static inline T* zvec_push_slot_##Name(zvec_##Name *v)                                      \
{                                                                                           \
    if (v->length >= v->capacity)                                                           \
    {                                                                                       \
        size_t new_cap = Z_GROWTH_FACTOR(v->capacity);                                      \
        if (zvec_reserve_##Name(v, new_cap) != Z_OK) return NULL;                           \
    }                                                                                       \
    return &v->data[v->length++];                                                           \
}                                                                                           \
                                                                                            \
/* Appends a value to the end of the vector. */                                             \
static inline int zvec_push_##Name(zvec_##Name *v, T value)                                 \
{                                                                                           \
    T *slot = zvec_push_slot_##Name(v);                                                     \
    if (!slot) return Z_ERR;                                                                \
    *slot = value;                                                                          \
    return Z_OK;                                                                            \
}                                                                                           \
                                                                                            \
/* Appends multiple items from a raw array to the vector. */                                \
static inline int zvec_extend_##Name(zvec_##Name *v, const T *items, size_t count)          \
{                                                                                           \
    if (v->length + count > v->capacity)                                                    \
    {                                                                                       \
        size_t new_cap = v->capacity;                                                       \
        if (new_cap == 0)                                                                   \
        {                                                                                   \
            new_cap = Z_GROWTH_FACTOR(0);                                                   \
        }                                                                                   \
        while (new_cap < v->length + count)                                                 \
        {                                                                                   \
            new_cap = Z_GROWTH_FACTOR(new_cap);                                             \
        }                                                                                   \
        if (zvec_reserve_##Name(v, new_cap) != Z_OK) return Z_ERR;                          \
    }                                                                                       \
    memcpy(v->data + v->length, items, count * sizeof(T));                                  \
    v->length += count;                                                                     \
    return Z_OK;                                                                            \
}                                                                                           \
                                                                                            \
/* Removes the last element (decrements length). Asserts if empty. */                       \
static inline void zvec_pop_##Name(zvec_##Name *v)                                          \
{                                                                                           \
    assert(v->length > 0 && "Popping empty vector");                                        \
    v->length--;                                                                            \
}                                                                                           \
                                                                                            \
/* Removes and returns the last element. Asserts if empty. */                               \
static inline T zvec_pop_get_##Name(zvec_##Name *v)                                         \
{                                                                                           \
    assert(v->length > 0 && "Vector is empty, cannot pop!");                                \
    return v->data[--v->length];                                                            \
}                                                                                           \
                                                                                            \
/* Reduces capacity to match the current length to save memory. */                          \
static inline void zvec_shrink_to_fit_##Name(zvec_##Name *v)                                \
{                                                                                           \
    if (v->length == 0)                                                                     \
    {                                                                                       \
        ZVEC_FREE(v->data);                                                                 \
        memset(v, 0, sizeof(zvec_##Name));                                                  \
        return;                                                                             \
    }                                                                                       \
    if (v->length == v->capacity) return;                                                   \
    T *new_data = (T*)ZVEC_REALLOC(v->data, v->length * sizeof(T));                         \
    if (!new_data) return;                                                                  \
    v->data = new_data;                                                                     \
    v->capacity = v->length;                                                                \
}                                                                                           \
                                                                                            \
/* Returns a pointer to the element at index, or NULL if out of bounds. */                  \
static inline T* zvec_at_##Name(zvec_##Name *v, size_t index)                               \
{                                                                                           \
    return (index < v->length) ? &v->data[index] : NULL;                                    \
}                                                                                           \
                                                                                            \
/* Returns the raw data pointer. */                                                         \
static inline T* zvec_data_##Name(zvec_##Name *v)                                           \
{                                                                                           \
    return v->data;                                                                         \
}                                                                                           \
                                                                                            \
/* Returns a pointer to the last element, or NULL if empty. */                              \
static inline T* zvec_last_##Name(zvec_##Name *v)                                           \
{                                                                                           \
    return (v->length > 0) ? &v->data[v->length - 1] : NULL;                                \
}                                                                                           \
                                                                                            \
/* Removes element at index, shifting subsequent elements left. O(N). */                    \
static inline void zvec_remove_##Name(zvec_##Name *v, size_t index)                         \
{                                                                                           \
    if (index >= v->length) return;                                                         \
    memmove(&v->data[index], &v->data[index + 1],                                           \
            (v->length - index - 1) * sizeof(T));                                           \
    v->length--;                                                                            \
}                                                                                           \
                                                                                            \
/* Removes element at index by swapping with the last element. O(1). */                     \
static inline void zvec_swap_remove_##Name(zvec_##Name *v, size_t index)                    \
{                                                                                           \
    if (index >= v->length) return;                                                         \
    v->data[index] = v->data[--v->length];                                                  \
}                                                                                           \
                                                                                            \
/* Sets the length to 0, logically clearing the vector. */                                  \
static inline void zvec_clear_##Name(zvec_##Name *v)                                        \
{                                                                                           \
    v->length = 0;                                                                          \
}                                                                                           \
                                                                                            \
/* Frees the underlying memory and resets the struct to zero. */                            \
static inline void zvec_free_##Name(zvec_##Name *v)                                         \
{                                                                                           \
    ZVEC_FREE(v->data);                                                                     \
    memset(v, 0, sizeof(zvec_##Name));                                                      \
}                                                                                           \
                                                                                            \
/* Reverses the order of elements in place. */                                              \
static inline void zvec_reverse_##Name(zvec_##Name *v)                                      \
{                                                                                           \
    if (v->length < 2) return;                                                              \
    size_t i = 0, j = v->length - 1;                                                        \
    while (i < j)                                                                           \
    {                                                                                       \
        T temp = v->data[i];                                                                \
        v->data[i] = v->data[j];                                                            \
        v->data[j] = temp;                                                                  \
        i++; j--;                                                                           \
    }                                                                                       \
}                                                                                           \
                                                                                            \
/* Sorts the vector using qsort and the provided comparison function. */                    \
static inline void zvec_sort_##Name(zvec_##Name *v, int (*compar)(const T *, const T *))    \
{                                                                                           \
    if (v->length > 1)                                                                      \
    {                                                                                       \
        int (*qsort_cmp)(const void *, const void *) =                                      \
            (int (*)(const void *, const void *))compar;                                    \
        qsort(v->data, v->length, sizeof(T), qsort_cmp);                                    \
    }                                                                                       \
}                                                                                           \
                                                                                            \
/* Performs a binary search for a key. Returns pointer to item or NULL. */                  \
static inline T* zvec_bsearch_##Name(zvec_##Name *v, const T *key,                          \
                                    int (*compar)(const T *, const T *))                    \
{                                                                                           \
    if (v->length == 0) return NULL;                                                        \
    int (*bs_cmp)(const void *, const void *) =                                             \
        (int (*)(const void *, const void *))compar;                                        \
    return (T*) bsearch(key, v->data, v->length, sizeof(T), bs_cmp);                        \
}                                                                                           \
                                                                                            \
/* Returns pointer to the first element that does not compare less than key. */             \
static inline T* zvec_lower_bound_##Name(zvec_##Name *v, const T *key,                      \
                                        int (*compar)(const T *, const T *))                \
{                                                                                           \
    size_t l = 0;                                                                           \
    size_t r = v->length;                                                                   \
    while (l < r)                                                                           \
    {                                                                                       \
        size_t mid = l + (r - l) / 2;                                                       \
        if (compar((const T*)&v->data[mid], key) < 0)                                       \
        {                                                                                   \
            l = mid + 1;                                                                    \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            r = mid;                                                                        \
        }                                                                                   \
    }                                                                                       \
    if (l == v->length) return NULL;                                                        \
    return &v->data[l];                                                                     \
}

/* C generic dispatch entries */
#define PUSH_ENTRY(T, Name)         zvec_##Name*: zvec_push_##Name,
#define PUSH_SLOT_ENTRY(T, Name)    zvec_##Name*: zvec_push_slot_##Name,
#define EXTEND_ENTRY(T, Name)       zvec_##Name*: zvec_extend_##Name,
#define RESERVE_ENTRY(T, Name)      zvec_##Name*: zvec_reserve_##Name,
#define IS_EMPTY_ENTRY(T, Name)     zvec_##Name*: zvec_is_empty_##Name,
#define AT_ENTRY(T, Name)           zvec_##Name*: zvec_at_##Name,
#define DATA_ENTRY(T, Name)         zvec_##Name*: zvec_data_##Name,
#define LAST_ENTRY(T, Name)         zvec_##Name*: zvec_last_##Name,
#define FREE_ENTRY(T, Name)         zvec_##Name*: zvec_free_##Name,
#define POP_ENTRY(T, Name)          zvec_##Name*: zvec_pop_##Name,
#define POP_GET_ENTRY(T, Name)      zvec_##Name*: zvec_pop_get_##Name,
#define SHRINK_ENTRY(T, Name)       zvec_##Name*: zvec_shrink_to_fit_##Name,
#define REMOVE_ENTRY(T, Name)       zvec_##Name*: zvec_remove_##Name,
#define SWAP_REM_ENTRY(T, Name)     zvec_##Name*: zvec_swap_remove_##Name,
#define CLEAR_ENTRY(T, Name)        zvec_##Name*: zvec_clear_##Name,
#define REVERSE_ENTRY(T, Name)      zvec_##Name*: zvec_reverse_##Name,
#define SORT_ENTRY(T, Name)         zvec_##Name*: zvec_sort_##Name,
#define BSEARCH_ENTRY(T, Name)      zvec_##Name*: zvec_bsearch_##Name,
#define LOWER_BOUND_ENTRY(T, Name)  zvec_##Name*: zvec_lower_bound_##Name,

#if Z_HAS_ZERROR
    #define RESERVE_SAFE_ENTRY(T, Name) zvec_##Name*: zvec_reserve_safe_##Name,
    #define PUSH_SAFE_ENTRY(T, Name)    zvec_##Name*: zvec_push_safe_##Name,
    #define POP_SAFE_ENTRY(T, Name)     zvec_##Name*: zvec_pop_safe_##Name,
    #define AT_SAFE_ENTRY(T, Name)      zvec_##Name*: zvec_at_safe_##Name,
    #define LAST_SAFE_ENTRY(T, Name)    zvec_##Name*: zvec_last_safe_##Name,
#endif

/* Registry & type generation */
#if defined(__has_include) && __has_include("z_registry.h")
    #include "z_registry.h"
#endif

#ifndef Z_AUTOGEN_VECS
    #define Z_AUTOGEN_VECS(X)
#endif

#ifndef REGISTER_ZVEC_TYPES
    #define REGISTER_ZVEC_TYPES(X)
#endif

// Combine all sources of types.
#define Z_ALL_VECS(X)       \
    Z_AUTOGEN_VECS(X)       \
    REGISTER_ZVEC_TYPES(X)

// Execute the generator for all registered types.
Z_ALL_VECS(ZVEC_GENERATE_IMPL)

/* C API Macros (using _Generic) */

// Helper to create a vector from literal args.
#define zvec_from(Name, ...) \
    zvec_from_array_##Name((zvec_T_##Name[])__VA_ARGS__, sizeof((zvec_T_##Name[])__VA_ARGS__) / sizeof(zvec_T_##Name))

// Initializes a zero-filled vector on the stack.
#define zvec_init(Name) zvec_init_capacity_##Name(0)

// Initializes a vector with pre-allocated capacity.
#define zvec_init_with_cap(Name, cap) zvec_init_capacity_##Name(cap)

// Auto-cleanup extension (GCC/Clang).
#if Z_HAS_CLEANUP
    #define zvec_autofree(Name)  Z_CLEANUP(zvec_free_##Name) zvec_##Name
#endif

// Generic Function Calls.
#define zvec_push(v, val)          _Generic((v), Z_ALL_VECS(PUSH_ENTRY)          default: 0)       (v, val)
#define zvec_push_slot(v)          _Generic((v), Z_ALL_VECS(PUSH_SLOT_ENTRY)     default: (void*)0)(v)
#define zvec_extend(v, arr, count) _Generic((v), Z_ALL_VECS(EXTEND_ENTRY)        default: 0)       (v, arr, count)
#define zvec_reserve(v, cap)       _Generic((v), Z_ALL_VECS(RESERVE_ENTRY)       default: 0)       (v, cap)
#define zvec_is_empty(v)           _Generic((v), Z_ALL_VECS(IS_EMPTY_ENTRY)      default: 0)       (v)
#define zvec_at(v, idx)            _Generic((v), Z_ALL_VECS(AT_ENTRY)            default: (void*)0)(v, idx)
#define zvec_data(v)               _Generic((v), Z_ALL_VECS(DATA_ENTRY)          default: (void*)0)(v)
#define zvec_last(v)               _Generic((v), Z_ALL_VECS(LAST_ENTRY)          default: (void*)0)(v)
#define zvec_free(v)               _Generic((v), Z_ALL_VECS(FREE_ENTRY)          default: (void)0) (v)
#define zvec_pop(v)                _Generic((v), Z_ALL_VECS(POP_ENTRY)           default: (void)0) (v)
#define zvec_pop_get(v)            _Generic((v), Z_ALL_VECS(POP_GET_ENTRY)       default: (void)0) (v)
#define zvec_shrink_to_fit(v)      _Generic((v), Z_ALL_VECS(SHRINK_ENTRY)        default: (void)0) (v)
#define zvec_remove(v, i)          _Generic((v), Z_ALL_VECS(REMOVE_ENTRY)        default: (void)0) (v, i)
#define zvec_swap_remove(v, i)     _Generic((v), Z_ALL_VECS(SWAP_REM_ENTRY)      default: (void)0) (v, i)
#define zvec_clear(v)              _Generic((v), Z_ALL_VECS(CLEAR_ENTRY)         default: (void)0) (v)
#define zvec_reverse(v)            _Generic((v), Z_ALL_VECS(REVERSE_ENTRY)       default: (void)0) (v)
#define zvec_sort(v, cmp)          _Generic((v), Z_ALL_VECS(SORT_ENTRY)          default: (void)0) (v, cmp)
#define zvec_bsearch(v, k, c)      _Generic((v), Z_ALL_VECS(BSEARCH_ENTRY)       default: (void*)0)(v, k, c)
#define zvec_lower_bound(v, k, c)  _Generic((v), Z_ALL_VECS(LOWER_BOUND_ENTRY)   default: (void*)0)(v, k, c)

// Helper Macros.
#define ZVEC_CAT(a, b) a##b
#define ZVEC_NAME(a, b) ZVEC_CAT(a, b)

// Iteration helper (C only).
#define zvec_foreach(v, iter) \
    for (size_t ZVEC_NAME(_i_, __LINE__) = 0; \
         ZVEC_NAME(_i_, __LINE__) < (v)->length && ((iter) = &(v)->data[ZVEC_NAME(_i_, __LINE__)]); \
         ++ZVEC_NAME(_i_, __LINE__))

/* Safe API Macros (conditioned on zerror.h). */
#if Z_HAS_ZERROR
    /* Dummy handler must accept variadic args to eat the extra file/line/func params */
    static inline zres zres_err_dummy(void* v, ...) 
    { 
        return zres_err(zerr_create(-1, "Unknown Vector Type")); 
    }

    #define zvec_reserve_safe(v, cap) \
        _Generic((v), Z_ALL_VECS(RESERVE_SAFE_ENTRY) default: zres_err_dummy)(v, cap, __FILE__, __LINE__, __func__)

    #define zvec_push_safe(v, val) \
        _Generic((v), Z_ALL_VECS(PUSH_SAFE_ENTRY) default: zres_err_dummy)(v, val, __FILE__, __LINE__, __func__)

    #define zvec_pop_safe(v) \
        _Generic((v), Z_ALL_VECS(POP_SAFE_ENTRY)  default: zres_err_dummy)(v, __FILE__, __LINE__, __func__)

    #define zvec_at_safe(v, i) \
        _Generic((v), Z_ALL_VECS(AT_SAFE_ENTRY)   default: zres_err_dummy)(v, i, __FILE__, __LINE__, __func__)

    #define zvec_last_safe(v) \
        _Generic((v), Z_ALL_VECS(LAST_SAFE_ENTRY) default: zres_err_dummy)(v, __FILE__, __LINE__, __func__)
#endif

/* Optional short names. */
#ifdef ZVEC_SHORT_NAMES
    /* Types. */
    #define vec(Name)           zvec_##Name
    
    /* Creation. */
    #define vec_init            zvec_init
    #define vec_init_with_cap   zvec_init_with_cap
    #define vec_from            zvec_from
    #define vec_autofree        zvec_autofree
    
    /* Operations. */
    #define vec_push            zvec_push
    #define vec_push_slot       zvec_push_slot
    #define vec_extend          zvec_extend
    #define vec_reserve         zvec_reserve
    #define vec_is_empty        zvec_is_empty
    #define vec_at              zvec_at
    #define vec_data            zvec_data
    #define vec_last            zvec_last
    #define vec_free            zvec_free
    #define vec_pop             zvec_pop
    #define vec_pop_get         zvec_pop_get
    #define vec_shrink_to_fit   zvec_shrink_to_fit
    #define vec_remove          zvec_remove
    #define vec_swap_remove     zvec_swap_remove
    #define vec_clear           zvec_clear
    #define vec_reverse         zvec_reverse
    #define vec_sort            zvec_sort
    #define vec_bsearch         zvec_bsearch
    #define vec_lower_bound     zvec_lower_bound
    
    /* Iteration. */
    #define vec_foreach         zvec_foreach
    
    /* Safe API. */
    #if Z_HAS_ZERROR
        #define vec_reserve_safe    zvec_reserve_safe
        #define vec_push_safe       zvec_push_safe
        #define vec_pop_safe        zvec_pop_safe
        #define vec_at_safe         zvec_at_safe
        #define vec_last_safe       zvec_last_safe
    #endif
#endif

/* C++ Trait specialization (executed after C code is defined). */
#ifdef __cplusplus
} // extern "C"

namespace z_vec
{
    // Macro to specialize the traits for generated types.
    #define ZVEC_CPP_TRAITS(T, Name)                                                                            \
        template<> struct traits<T>                                                                             \
        {                                                                                                       \
            using c_type = ::zvec_##Name;                                                                       \
            static inline void init(c_type& v) { v = ::zvec_init_capacity_##Name(0); }                          \
            static inline void init_cap(c_type& v, size_t c) { v = ::zvec_init_capacity_##Name(c); }            \
            static inline void free(c_type& v) { ::zvec_free_##Name(&v); }                                      \
            static inline void push(c_type& v, T val) { ::zvec_push_##Name(&v, val); }                          \
            static inline void extend(c_type& v, const T* arr, size_t n) { ::zvec_extend_##Name(&v, arr, n); }  \
            static inline void reserve(c_type& v, size_t n) { ::zvec_reserve_##Name(&v, n); }                   \
            static inline void pop(c_type& v) { ::zvec_pop_##Name(&v); }                                        \
            static inline T    pop_get(c_type& v) { return ::zvec_pop_get_##Name(&v); }                         \
            static inline void remove(c_type& v, size_t i) { ::zvec_remove_##Name(&v, i); }                     \
            static inline void swap_remove(c_type& v, size_t i) { ::zvec_swap_remove_##Name(&v, i); }           \
            static inline void clear(c_type& v) { ::zvec_clear_##Name(&v); }                                    \
            static inline void shrink(c_type& v) { ::zvec_shrink_to_fit_##Name(&v); }                           \
            static inline void reverse(c_type& v) { ::zvec_reverse_##Name(&v); }                                \
        };

    // Execute the trait specialization for all registered types.
    Z_ALL_VECS(ZVEC_CPP_TRAITS)
} // namespace z_vec
#endif // __cplusplus

#endif // ZVEC_H
