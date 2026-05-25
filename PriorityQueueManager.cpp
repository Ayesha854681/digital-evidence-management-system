#include "PriorityQueueManager.h"

// O(log n) - Insert with priority
void PriorityQueueManager::assignPriority(Evidence e) {
    caseQueue.push(CaseNode(e));
    cout << "Evidence added to priority queue!\n";
}

// O(1) - Get highest priority
Evidence PriorityQueueManager::getHighestPriority() const {
    if (caseQueue.empty()) {
        return Evidence();
    }
    return caseQueue.top().evidence;
}

// O(log n) - Remove highest priority
void PriorityQueueManager::processNextCase() {
    if (!caseQueue.empty()) {
        caseQueue.pop();
        cout << "Highest priority case processed!\n";
    }
    else {
        cout << "Queue is empty!\n";
    }
}

// O(n log n) - Display all in priority order
void PriorityQueueManager::displayAll() {
    if (caseQueue.empty()) {
        cout << "\nNo cases in priority queue.\n";
        return;
    }

    cout << "\n" << string(100, '=') << "\n";
    cout << left << setw(14) << "Evidence ID" << setw(12) << "Case ID"
        << setw(20) << "Description" << setw(14) << "Hash"
        << setw(12) << "Verified" << setw(10) << "Priority" << "Timestamp" << endl;
    cout << string(100, '=') << endl;

    priority_queue<CaseNode> temp = caseQueue;
    while (!temp.empty()) {
        temp.top().evidence.display();
        temp.pop();
    }
    cout << string(100, '=') << "\n";
}

// O(1)
int PriorityQueueManager::getCount() const {
    return static_cast<int>(caseQueue.size());
}

// O(1)
bool PriorityQueueManager::isEmpty() const {
    return caseQueue.empty();
}

// Display stats
void PriorityQueueManager::displayStats() const {
    cout << "\n--- Priority Queue Statistics ---\n";
    cout << "Total Cases in Queue: " << getCount() << "\n\n";
}