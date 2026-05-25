#pragma once
#include "Evidence.h"
#include "HashCalculator.h"
#include <unordered_map>
#include <vector>

class HashMapManager {
private:
    unordered_map<string, Evidence> evidenceMap;

public:
    // Registration & Storage
    void registerEvidence(Evidence e);
    void registerEvidenceWithFile(Evidence e);
    void updateEvidence(string id, Evidence e);
    Evidence* retrieveEvidence(string id);
    void removeEvidence(string id);
    void displayAll() const;

    // Verification - ENHANCED
    bool verifyEvidence(string id);
    bool verifyFileIntegrity(string id);
    void recalculateHash(string id);
    int getVerifiedCount() const;
    int getUnverifiedCount() const;
    int getIntegrityValidCount() const;
    int getIntegrityInvalidCount() const;
    void updateHash(string id, string newHash);

    // Search
    vector<Evidence> searchByCase(string caseID) const;

    // NEW: Get all evidence as vector (for advanced search)
    // DEFINED ONLY ONCE - inline implementation
    vector<Evidence> getAllEvidenceVector() const {
        vector<Evidence> result;
        for (const auto& pair : evidenceMap) {
            result.push_back(pair.second);
        }
        return result;
    }

    // Utility
    int getTotalCount() const;
    void clearAll();
    void displayStats() const;
};