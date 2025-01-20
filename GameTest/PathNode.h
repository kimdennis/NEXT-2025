#pragma once
#include <vector>

//PathNode.h is a helper class for the pathfinding algorithm in the game

struct PathNode {
    float x, y;
    std::vector<int> connections;  // Indices of connected nodes
    bool isControlPoint;           // Whether this is a main control point
    
    PathNode(float _x, float _y, bool control = false) 
        : x(_x), y(_y), isControlPoint(control) {}
};