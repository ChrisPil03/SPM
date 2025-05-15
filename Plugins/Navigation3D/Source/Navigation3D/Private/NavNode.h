#pragma once

#include "CoreMinimal.h" // Usually good to have for UE types like FIntVector
#include <vector>
#include <limits>       // For std::numeric_limits

// Represents a single node in the 3D navigation grid
struct NavNode
{
    FIntVector Coordinates = FIntVector::ZeroValue;
    std::vector<NavNode*> Neighbors;
    bool bIsTraversable = true;

    // --- Per-search A* data ---
    // This data will be valid only for the current ongoing FindPath operation,
    // identified by comparing SearchID_Pathfinding with ANavigationVolume3D::CurrentPathfindingSearchID.
    float FScore_Pathfinding = std::numeric_limits<float>::max();
    float GScore_Pathfinding = std::numeric_limits<float>::max();
    NavNode* CameFrom_Pathfinding = nullptr;
    uint32_t SearchID_Pathfinding = 0; // 0 can mean "not yet visited by any search" or "data is stale"

    NavNode() = default;

    // Optional: Add a reset function if you prefer not to check SearchID everywhere
    // void ResetForPathfinding() {
    //     FScore_Pathfinding = std::numeric_limits<float>::max();
    //     GScore_Pathfinding = std::numeric_limits<float>::max();
    //     CameFrom_Pathfinding = nullptr;
    //     // SearchID_Pathfinding would be set by the pathfinder
    // }
};

// Comparison struct for the A* open set (used with std::priority_queue or std::multiset)
// std::priority_queue is a max-heap by default, so for a min-heap (lowest FScore first),
// the comparator needs to return true if 'lhs' has a GREATER FScore than 'rhs'.
struct NodeCompare
{
    bool operator()(const NavNode* lhs, const NavNode* rhs) const
    {
        if (!lhs) return true;  // Null is considered "greater" (pushed further down in a min-priority_queue)
        if (!rhs) return false; // Non-null is considered "lesser"

        // Primary sort key: FScore_Pathfinding
        // For std::priority_queue (min-heap behavior):
        if (lhs->FScore_Pathfinding > rhs->FScore_Pathfinding) return true;
        if (lhs->FScore_Pathfinding < rhs->FScore_Pathfinding) return false;

        // Secondary sort key (tie-breaker): Memory address for stability
        return lhs > rhs; // Consistent tie-breaker for priority_queue
    }
};