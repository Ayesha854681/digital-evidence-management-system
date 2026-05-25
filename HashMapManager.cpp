#include "HashMapManager.h"

// O(1) - Insert evidence (manual hash)
void HashMapManager::registerEvidence(Evidence e) {
    evidenceMap[e.id] = e;
    cout << "Evidence registered successfully!\n";
}

// NEW: O(1) + O(file_size) - Insert evidence with automatic hash calculation
void HashMapManager::registerEvidenceWithFile(Evidence e) {
    if (e.filePath.empty()) {
        cout << "Error: File path not specified!\n";
        return;
    }

    // Check if file exists
    if (!HashCalculator::fileExists(e.filePath)) {
        cout << "Error: File does not exist: " << e.filePath << "\n";
        return;
    }

    // Calculate hash automatically
    cout << "Calculating SHA-256 hash for file...\n";
    string calculatedHash = HashCalculator::calculateFileHash(e.filePath);

    if (calculatedHash.empty()) {
        cout << "Error: Failed to calculate hash!\n";
        return;
    }

    e.hash = calculatedHash;

    // Get actual file size
    long long actualSize = HashCalculator::getFileSize(e.filePath);
    if (actualSize >= 0) {
        e.fileSize = actualSize;
    }

    e.integrityValid = true;  // Initial registration is valid

    // IMPORTANT: Store in map
    evidenceMap[e.id] = e;

    cout << "[OK] Evidence registered successfully!\n";
    cout << "[OK] SHA-256 Hash: " << calculatedHash << "\n";
    cout << "[OK] File Size: " << actualSize << " bytes\n";
}

// O(1) - Update evidence
void HashMapManager::updateEvidence(string id, Evidence e) {
    if (evidenceMap.find(id) != evidenceMap.end()) {
        evidenceMap[id] = e;
        cout << "Evidence updated successfully!\n";
    }
    else {
        cout << "Evidence not found!\n";
    }
}

// O(1) - Retrieve evidence
Evidence* HashMapManager::retrieveEvidence(string id) {
    auto it = evidenceMap.find(id);
    if (it != evidenceMap.end()) {
        return &it->second;
    }
    return nullptr;
}

// O(1) - Remove evidence
void HashMapManager::removeEvidence(string id) {
    if (evidenceMap.erase(id)) {
        cout << "Evidence removed successfully!\n";
    }
    else {
        cout << "Evidence not found!\n";
    }
}

// O(n) - Display all
void HashMapManager::displayAll() const {
    if (evidenceMap.empty()) {
        cout << "\nNo evidence stored.\n";
        return;
    }
    cout << "\n" << string(120, '=') << "\n";
    cout << left << setw(14) << "Evidence ID" << setw(12) << "Case ID"
        << setw(20) << "Description" << setw(16) << "Hash"
        << setw(12) << "Verified" << setw(12) << "Integrity"
        << setw(10) << "Priority" << "Timestamp" << endl;
    cout << string(120, '=') << endl;
    for (auto& p : evidenceMap) {
        p.second.display();
    }
    cout << string(120, '=') << "\n";
}

// O(1) - Mark as verified (simple flag)
bool HashMapManager::verifyEvidence(string id) {
    auto it = evidenceMap.find(id);
    if (it != evidenceMap.end()) {
        it->second.verified = true;
        cout << "Evidence marked as verified!\n";
        return true;
    }
    else {
        cout << "Evidence not found!\n";
        return false;
    }
}

// NEW: O(1) + O(file_size) - Verify actual file integrity
bool HashMapManager::verifyFileIntegrity(string id) {
    auto it = evidenceMap.find(id);
    if (it == evidenceMap.end()) {
        cout << "Error: Evidence not found!\n";
        return false;
    }

    Evidence& e = it->second;

    // Check if file path exists
    if (e.filePath.empty()) {
        cout << "Error: No file path specified for this evidence!\n";
        return false;
    }

    // Check if file exists
    if (!HashCalculator::fileExists(e.filePath)) {
        cout << "WARNING: File not found: " << e.filePath << "\n";
        e.integrityValid = false;
        return false;
    }

    // Check if original hash exists
    if (e.hash.empty()) {
        cout << "Error: No original hash to compare against!\n";
        return false;
    }

    cout << "Verifying file integrity for " << id << "...\n";
    cout << "Original Hash: " << e.hash << "\n";

    // Calculate current hash
    string currentHash = HashCalculator::calculateFileHash(e.filePath);

    if (currentHash.empty()) {
        cout << "Error: Failed to calculate current hash!\n";
        return false;
    }

    cout << "Current Hash:  " << currentHash << "\n";

    // Compare hashes
    bool isValid = HashCalculator::verifyFileIntegrity(e.filePath, e.hash);

    if (isValid) {
        cout << "\n[OK] INTEGRITY VALID - File has NOT been tampered with!\n";
        e.integrityValid = true;
        e.verified = true;
    }
    else {
        cout << "\n[!!] INTEGRITY VIOLATION - File has been MODIFIED or CORRUPTED!\n";
        cout << "[!!] This evidence may not be admissible in court!\n";
        e.integrityValid = false;
    }

    return isValid;
}

// NEW: Recalculate hash from file
void HashMapManager::recalculateHash(string id) {
    auto it = evidenceMap.find(id);
    if (it == evidenceMap.end()) {
        cout << "Evidence not found!\n";
        return;
    }

    Evidence& e = it->second;

    if (e.filePath.empty()) {
        cout << "Error: No file path specified!\n";
        return;
    }

    if (!HashCalculator::fileExists(e.filePath)) {
        cout << "Error: File not found: " << e.filePath << "\n";
        return;
    }

    cout << "Recalculating SHA-256 hash...\n";
    string newHash = HashCalculator::calculateFileHash(e.filePath);

    if (newHash.empty()) {
        cout << "Error: Failed to calculate hash!\n";
        return;
    }

    cout << "Old Hash: " << e.hash << "\n";
    cout << "New Hash: " << newHash << "\n";

    e.hash = newHash;
    e.integrityValid = true;

    cout << "[OK] Hash updated successfully!\n";
}

// O(n) - Count verified
int HashMapManager::getVerifiedCount() const {
    int count = 0;
    for (auto& p : evidenceMap) {
        if (p.second.verified) count++;
    }
    return count;
}

// O(n) - Count unverified
int HashMapManager::getUnverifiedCount() const {
    int count = 0;
    for (auto& p : evidenceMap) {
        if (!p.second.verified) count++;
    }
    return count;
}

// NEW: Count integrity valid
int HashMapManager::getIntegrityValidCount() const {
    int count = 0;
    for (auto& p : evidenceMap) {
        if (p.second.integrityValid) count++;
    }
    return count;
}

// NEW: Count integrity invalid
int HashMapManager::getIntegrityInvalidCount() const {
    int count = 0;
    for (auto& p : evidenceMap) {
        if (!p.second.integrityValid && !p.second.hash.empty()) count++;
    }
    return count;
}

// O(1) - Update hash manually
void HashMapManager::updateHash(string id, string newHash) {
    auto it = evidenceMap.find(id);
    if (it != evidenceMap.end()) {
        it->second.hash = newHash;
        cout << "Hash updated successfully!\n";
    }
    else {
        cout << "Evidence not found!\n";
    }
}

// O(n) - Search by case
vector<Evidence> HashMapManager::searchByCase(string caseID) const {
    vector<Evidence> results;
    for (auto& p : evidenceMap) {
        if (p.second.caseID == caseID) {
            results.push_back(p.second);
        }
    }
    return results;
}

// O(1) - Get count
int HashMapManager::getTotalCount() const {
    return static_cast<int>(evidenceMap.size());
}

// Clear all
void HashMapManager::clearAll() {
    evidenceMap.clear();
    cout << "All evidence cleared!\n";
}
// Display stats - ENHANCED
void HashMapManager::displayStats() const {
    cout << "\n========================================\n";
    cout << "     HASH MAP STATISTICS                \n";
    cout << "========================================\n";
    cout << " Total Evidence:        " << setw(15) << getTotalCount() << "\n";
    cout << " Verified:              " << setw(15) << getVerifiedCount() << "\n";
    cout << " Unverified:            " << setw(15) << getUnverifiedCount() << "\n";
    cout << " Integrity Valid:       " << setw(15) << getIntegrityValidCount() << "\n";
    cout << " Integrity Issues:      " << setw(15) << getIntegrityInvalidCount() << "\n";
    cout << "========================================\n\n";
}