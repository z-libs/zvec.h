
#include <iostream>
#include <iomanip>

struct Point 
{
    float x, y;
};

#include "zvec.h"

DEFINE_VEC_TYPE(int, Int)
DEFINE_VEC_TYPE(Point, Point)

int main() 
{
    std::cout << "=> Integers\n";
    
    z_vec::vector<int> nums = {10, 20, 30};

    nums.push_back(40);

    std::cout << "Values: ";
    for (int x : nums) 
    {
        std::cout << x << " ";
    }
    std::cout << "\n\n";

    std::cout << "=> Points\n";
    
    z_vec::vector<Point> points;
   
    points.push_back({1.5f, 2.5f});
    points.push_back({3.0f, 4.0f});

    for (const auto& p : points) 
    {
        std::cout << "Point: {x: " << p.x << ", y: " << p.y << "}\n";
    }

    try 
    {
        points.at(0).x = 99.9f;
        std::cout << "Modified Point 0 x: " << points[0].x << "\n";
    } catch (const std::exception& e) 
    {
        std::cout << "Error: " << e.what() << "\n";
    }

    return 0;
}
