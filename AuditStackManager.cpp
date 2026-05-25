#define _CRT_SECURE_NO_WARNINGS
#include "AuditStackManager.h"

// O(1) - Push action
void AuditStackManager::logAction(string action, string eID, string details) {
    auditStack.push(AuditLog(action, eID, details));
    cout << "Action logged!\n";
}

// O(1) - Peek top
AuditLog AuditStackManager::viewMostRecent() const {
    if (!auditStack.empty()) {
        return auditStack.top();
    }
    return AuditLog();
}

// O(n) - Display all
void AuditStackManager::viewComplete() const {
    if (auditStack.empty()) {
        cout << "\nNo audit actions.\n";
        return;
    }

    cout << "\n" << string(90, '=') << "\n";
    cout << left << setw(16) << "Timestamp" << setw(12) << "Action"
        << setw(14) << "Evidence ID" << setw(30) << "Details" << endl;
    cout << string(90, '=') << endl;

    stack<AuditLog> temp = auditStack;
    while (!temp.empty()) {
        temp.top().display();
        temp.pop();
    }
    cout << string(90, '=') << "\n";
}

// O(1) - Pop
void AuditStackManager::undoLastAction() {
    if (!auditStack.empty()) {
        auditStack.pop();
        cout << "Last action undone!\n";
    }
    else {
        cout << "No actions to undo!\n";
    }
}

// O(n) - Search by evidence
vector<AuditLog> AuditStackManager::getActionsByEvidence(string eID) const {
    vector<AuditLog> results;
    stack<AuditLog> temp = auditStack;
    while (!temp.empty()) {
        if (temp.top().evidenceID == eID) {
            results.push_back(temp.top());
        }
        temp.pop();
    }
    return results;
}

// O(1)
int AuditStackManager::getCount() const {
    return static_cast<int>(auditStack.size());
}

// O(1)
bool AuditStackManager::isEmpty() const {
    return auditStack.empty();
}

// Clear all
void AuditStackManager::clearAll() {
    stack<AuditLog> empty;
    auditStack = empty;
    cout << "Audit trail cleared!\n";
}

// Display stats
void AuditStackManager::displayStats() const {
    cout << "\n--- Audit Stack Statistics ---\n";
    cout << "Total Actions Logged: " << getCount() << "\n\n";
}