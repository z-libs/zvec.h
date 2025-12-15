#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stdexcept>

struct Vec2 
{ 
    float x, y; 
    
    // Operator overload for std::find / assertions.
    bool operator==(const Vec2& other) const 
    {
        return x == other.x && y == other.y;
    }
};

#define REGISTER_ZVEC_TYPES(X) \
    X(int, Int)                \
    X(Vec2, Vec2)

#include "zvec.h"

#define TEST(name) printf("[TEST] %-40s", name);
#define PASS() std::cout << "\033[0;32mPASS\033[0m\n";

void test_constructors() 
{
    TEST("Constructors (Default, Cap, InitList)");

    // Default.
    z_vec::vector<int> v1;
    assert(v1.empty());
    assert(v1.capacity() == 0);

    // Capacity.
    z_vec::vector<int> v2(100);
    assert(v2.empty());
    assert(v2.capacity() == 100);

    // Initializer List.
    z_vec::vector<int> v3 = {1, 2, 3, 4, 5};
    assert(v3.size() == 5);
    assert(v3[0] == 1);
    assert(v3[4] == 5);

    PASS();
}

void test_rule_of_five() 
{
    TEST("Rule of 5 (Copy/Move Semantics)");

    // Copy Constructor (Deep Copy).
    z_vec::vector<int> original = {10, 20, 30};
    z_vec::vector<int> copy = original;
    
    assert(copy.size() == 3);
    assert(copy[0] == 10);
    assert(copy.data() != original.data()); // Must have distinct memory.
    
    // Modify copy, ensure original is untouched.
    copy[0] = 999;
    assert(original[0] == 10);

    // Move Constructor (Pointer Stealing).
    int* original_ptr = original.data();
    z_vec::vector<int> moved = std::move(original);

    assert(moved.data() == original_ptr); // Must steal pointer.
    assert(moved.size() == 3);
    assert(original.data() == nullptr);   // Source must be empty.
    assert(original.size() == 0);

    // Move Assignment.
    z_vec::vector<int> assigned;
    assigned = std::move(moved);
    assert(assigned.data() == original_ptr);
    assert(assigned.size() == 3);

    PASS();
}

void test_stl_interop() 
{
    TEST("STL Compatibility (Iterators, Sort)");

    z_vec::vector<int> v = {50, 10, 40, 30, 20};

    // Range-based for loop.
    int count = 0;
    for (int x : v) 
    {
        count++;
        (void)x;
    }
    assert(count == 5);

    // std::sort (Works because iterators are pointers).
    std::sort(v.begin(), v.end());

    assert(v[0] == 10);
    assert(v[4] == 50);

    // std::find.
    auto it = std::find(v.begin(), v.end(), 30);
    assert(it != v.end());
    assert(*it == 30);
    assert(std::distance(v.begin(), it) == 2);

    PASS();
}

void test_access_modifiers() 
{
    TEST("Access & Modifiers (Push, Pop, At)");

    z_vec::vector<Vec2> points;
    
    // Push Back.
    points.push_back({1.0f, 2.0f});
    points.push_back({3.0f, 4.0f});

    assert(points.size() == 2);
    assert(points.back().y == 4.0f);

    // Bounds checking (at).
    try {
        auto val = points.at(100);
        (void)val;
        assert(false && "Should have thrown out_of_range");
    } catch (const std::out_of_range& e) 
    {
        // Success.
    }

    // Pop Back.
    points.pop_back();
    assert(points.size() == 1);
    assert(points.back().x == 1.0f);

    // Clear.
    points.clear();
    assert(points.empty());
    assert(points.capacity() > 0); // Capacity retained

    PASS();
}

// Ensure const-correctness works.
void const_reader(const z_vec::vector<int>& v) 
{
    assert(!v.empty());
    assert(v[0] == 100);
    // v[0] = 5; // Should compile error if uncommented.
}

void test_const_correctness() 
{
    TEST("Const Correctness");
    
    z_vec::vector<int> v = {100};
    const_reader(v);
    
    PASS();
}

int main() 
{
    std::cout << "=> Running tests (zvec.h, cpp).\n";
    
    test_constructors();
    test_rule_of_five();
    test_stl_interop();
    test_access_modifiers();
    test_const_correctness();

    std::cout << "=> All tests passed successfully.\n";
    return 0;
}
