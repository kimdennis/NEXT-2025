#pragma once
#include <vector>

struct PathNode {
    float x, y;
    std::vector<int> connections;  // Indices of connected nodes
    bool isControlPoint;           // Whether this is a main control point
    
    PathNode(float _x, float _y, bool control = false) 
        : x(_x), y(_y), isControlPoint(control) {}
};