#pragma once
#include "Evidence.h"
#include <queue>
#include <vector>

struct CaseNode {
    Evidence evidence;
    CaseNode(Evidence e) : evidence(e) {}
    bool operator<(const CaseNode& other) const {
        return static_cast<int>(evidence.priority) <
            static_cast<int>(other.evidence.priority);
    }
};

class PriorityQueueManager {
private:
    priority_queue<CaseNode> caseQueue;

public:
    // Priority Queue operations
    void assignPriority(Evidence e);
    Evidence getHighestPriority() const;
    void processNextCase();
    void displayAll();

    // Utility
    int getCount() const;
    bool isEmpty() const;
    void displayStats() const;
};