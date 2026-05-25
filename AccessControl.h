#ifndef ACCESSCONTROL_H
#define ACCESSCONTROL_H

#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>
using namespace std;

enum class UserRole {
    ADMIN,          // Full access - delete, modify, configure
    INVESTIGATOR,   // Add, modify, verify evidence
    ANALYST,        // View and search only
    AUDITOR         // View audit trail and reports only
};

struct User {
    string username;
    string passwordHash;
    UserRole role;
    bool isActive;
    string fullName;

    User() : role(UserRole::ANALYST), isActive(false) {}
    User(string uname, string pHash, UserRole r, string fname)
        : username(uname), passwordHash(pHash), role(r), isActive(true), fullName(fname) {}
};

class AccessControlManager {
private:
    User currentUser;
    unordered_map<string, User> users;
    bool isLoggedIn;

    // Simple hash function (in production, use SHA-256)
    string hashPassword(const string& password) {
        hash<string> hasher;
        return to_string(hasher(password + "SALT_2025"));
    }

public:
    AccessControlManager() : isLoggedIn(false) {
        // Initialize default users
        users["admin"] = User("admin", hashPassword("admin123"), UserRole::ADMIN, "System Administrator");
        users["inv1"] = User("inv1", hashPassword("inv123"), UserRole::INVESTIGATOR, "John Investigator");
        users["analyst1"] = User("analyst1", hashPassword("ana123"), UserRole::ANALYST, "Jane Analyst");
        users["audit1"] = User("audit1", hashPassword("audit123"), UserRole::AUDITOR, "Audit Officer");
    }

    bool login(const string& username, const string& password) {
        auto it = users.find(username);
        if (it != users.end() && it->second.isActive) {
            if (it->second.passwordHash == hashPassword(password)) {
                currentUser = it->second;
                isLoggedIn = true;
                cout << "\n✓ Login successful! Welcome, " << currentUser.fullName << "\n";
                return true;
            }
        }
        cout << "\n✗ Login failed! Invalid credentials.\n";
        return false;
    }

    void logout() {
        isLoggedIn = false;
        cout << "\n✓ Logged out successfully.\n";
    }

    bool checkLoggedIn() const {
        if (!isLoggedIn) {
            cout << "\n[ERROR] You must be logged in to perform this action!\n";
        }
        return isLoggedIn;
    }

    bool hasPermission(const string& action) {
        if (!isLoggedIn) return false;

        UserRole role = currentUser.role;

        if (action == "DELETE" || action == "CLEAR" || action == "RESET") {
            return role == UserRole::ADMIN;
        }
        if (action == "MODIFY" || action == "VERIFY" || action == "REGISTER") {
            return role == UserRole::ADMIN || role == UserRole::INVESTIGATOR;
        }
        if (action == "VIEW" || action == "SEARCH") {
            return true;  // All logged-in users can view
        }
        if (action == "AUDIT") {
            return role == UserRole::ADMIN || role == UserRole::AUDITOR;
        }
        return false;
    }

    string getCurrentUsername() const { return currentUser.username; }
    string getCurrentFullName() const { return currentUser.fullName; }
    UserRole getCurrentRole() const { return currentUser.role; }

    string getRoleString() const {
        switch (currentUser.role) {
        case UserRole::ADMIN: return "Administrator";
        case UserRole::INVESTIGATOR: return "Investigator";
        case UserRole::ANALYST: return "Analyst";
        case UserRole::AUDITOR: return "Auditor";
        default: return "Unknown";
        }
    }

    void displayCurrentUser() const {
        if (isLoggedIn) {
            cout << "\n╔══════════════════════════════════════╗\n";
            cout << "║       CURRENT USER SESSION           ║\n";
            cout << "╠══════════════════════════════════════╣\n";
            cout << "║ User: " << left << setw(30) << currentUser.fullName << "║\n";
            cout << "║ Role: " << left << setw(30) << getRoleString() << "║\n";
            cout << "╚══════════════════════════════════════╝\n";
        }
        else {
            cout << "\n[INFO] No user logged in.\n";
        }
    }
};

#endif