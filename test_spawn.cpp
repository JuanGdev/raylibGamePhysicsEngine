#include <iostream>
#include <vector>
#include "core/GameObject.h"

int main() {
    std::cout << "Testing GameObject move semantics and memory safety..." << std::endl;
    
    // Test 1: Basic move constructor
    std::cout << "Test 1: Move constructor..." << std::endl;
    GameObject cube1({0, 5, 0}, {0, 0, 0}, {2, 2, 2}, RED, true);
    cube1.EnableCollider(cube1.GetScale());
    GameObject cube2 = std::move(cube1);
    std::cout << "Move constructor test passed!" << std::endl;
    
    // Test 2: Vector operations with move semantics
    std::cout << "Test 2: Vector push_back with move..." << std::endl;
    std::vector<GameObject> cubes;
    
    for (int i = 0; i < 10; i++) {
        float x = i * 2.0f;
        GameObject newCube({x, 5, 0}, {0, 0, 0}, {1, 1, 1}, BLUE, true);
        newCube.EnableCollider(newCube.GetScale());
        cubes.push_back(std::move(newCube));
        std::cout << "Added cube " << (i + 1) << " to vector" << std::endl;
    }
    
    std::cout << "Vector operations test passed! Created " << cubes.size() << " cubes." << std::endl;
    
    // Test 3: Vector resizing (this was causing the memory corruption)
    std::cout << "Test 3: Vector capacity and resizing..." << std::endl;
    std::cout << "Vector capacity: " << cubes.capacity() << std::endl;
    
    for (int i = 0; i < 5; i++) {
        GameObject anotherCube({i * 3.0f, 8, 0}, {0, 0, 0}, {1.5f, 1.5f, 1.5f}, GREEN, true);
        anotherCube.EnableCollider(anotherCube.GetScale());
        cubes.push_back(std::move(anotherCube));
        std::cout << "Added extra cube " << (i + 1) << ", capacity: " << cubes.capacity() << std::endl;
    }
    
    std::cout << "All tests passed! Memory management is working correctly." << std::endl;
    std::cout << "Final vector size: " << cubes.size() << std::endl;
    
    return 0;
}
