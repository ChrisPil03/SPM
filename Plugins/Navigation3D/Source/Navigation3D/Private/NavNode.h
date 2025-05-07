#pragma once

#include "CoreMinimal.h"
#include <vector> // Or use TArray if preferred and update usage below

// Represents a single node in the 3D navigation grid
struct NavNode
{
    FIntVector Coordinates = FIntVector::ZeroValue; // Grid coordinates (X, Y, Z)

    // Using std::vector as per your original code. TArray<NavNode*> could also be used.
    std::vector<NavNode*> Neighbors;

    // A* score (Cost from start + Heuristic estimate to end)
    float FScore = FLT_MAX;

    // Pre-calculated flag indicating if this node is blocked by an obstacle
    bool bIsTraversable = true;

    // Default constructor needed for TArray initialization
    NavNode() = default;
};

// Comparison struct for the A* open set (used with std::multiset)
// Sorts primarily by FScore, uses memory address as a stable tie-breaker.
struct NodeCompare
{
    bool operator()(const NavNode* lhs, const NavNode* rhs) const
    {
        // Handle potential null pointers gracefully if they could ever enter the set
        if (!lhs) return false; // Null is considered "greater"
        if (!rhs) return true;  // Non-null is considered "lesser"

        // Primary sort key: FScore
        if (lhs->FScore < rhs->FScore) return true;
        if (lhs->FScore > rhs->FScore) return false;

        // Secondary sort key (tie-breaker): Memory address for stability
        return lhs < rhs;
    }
};