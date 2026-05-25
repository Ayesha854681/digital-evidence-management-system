#pragma once
#include "Evidence.h"
#include <stack>
#include <vector>

class AuditStackManager {
private:
    stack<AuditLog> auditStack;

public:
    // Stack operations
    void logAction(string action, string eID, string details);
    AuditLog viewMostRecent() const;
    void viewComplete() const;
    void undoLastAction();

    // Search
    vector<AuditLog> getActionsByEvidence(string eID) const;

    // Utility
    int getCount() const;
    bool isEmpty() const;
    void clearAll();
    void displayStats() const;
};