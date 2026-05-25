#include "AdvancedSearch.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>  // ADD THIS LINE - Required for transform() and sort()

using namespace std;

// Convert string to lowercase
string AdvancedSearch::toLower(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// Case-insensitive substring search
bool AdvancedSearch::containsIgnoreCase(const string& str, const string& substr) {
    return toLower(str).find(toLower(substr)) != string::npos;
}

// Check if evidence matches all search criteria
bool AdvancedSearch::matchesCriteria(const Evidence& e, const SearchCriteria& c) {
    // Evidence ID match (exact)
    if (!c.evidenceID.empty() && e.id != c.evidenceID)
        return false;

    // Case ID match (exact)
    if (!c.caseID.empty() && e.caseID != c.caseID)
        return false;

    // Keyword search in description (case-insensitive)
    if (!c.keyword.empty() && !containsIgnoreCase(e.description, c.keyword))
        return false;

    // Collector name search (case-insensitive)
    if (!c.collectorName.empty() && !containsIgnoreCase(e.metadata.collectorName, c.collectorName))
        return false;

    // Source type search (case-insensitive)
    if (!c.sourceType.empty() && !containsIgnoreCase(e.metadata.sourceType, c.sourceType))
        return false;

    // Verified status filter
    if (c.verifiedOnly && !e.verified)
        return false;

    // Integrity valid filter
    if (c.integrityValidOnly && !e.integrityValid)
        return false;

    // Priority range filter
    if (e.priority < c.minPriority || e.priority > c.maxPriority)
        return false;

    // Date range filter
    if (c.startDate > 0 && e.timestamp < c.startDate)
        return false;

    if (c.endDate > 0 && e.timestamp > c.endDate)
        return false;

    return true;
}

// Main search function
vector<Evidence> AdvancedSearch::search(const HashMapManager& hashMap, const SearchCriteria& criteria) {
    vector<Evidence> results;
    vector<Evidence> allEvidence = hashMap.getAllEvidenceVector();

    for (const auto& e : allEvidence) {
        if (matchesCriteria(e, criteria)) {
            results.push_back(e);
        }
    }

    return results;
}

// Search by keyword only
vector<Evidence> AdvancedSearch::searchByKeyword(const HashMapManager& hashMap, const string& keyword) {
    SearchCriteria criteria;
    criteria.keyword = keyword;
    return search(hashMap, criteria);
}

// Search by priority only
vector<Evidence> AdvancedSearch::searchByPriority(const HashMapManager& hashMap, Priority priority) {
    SearchCriteria criteria;
    criteria.minPriority = priority;
    criteria.maxPriority = priority;
    return search(hashMap, criteria);
}

// Search by collector only
vector<Evidence> AdvancedSearch::searchByCollector(const HashMapManager& hashMap, const string& collector) {
    SearchCriteria criteria;
    criteria.collectorName = collector;
    return search(hashMap, criteria);
}

// Search verified evidence only
vector<Evidence> AdvancedSearch::searchVerifiedOnly(const HashMapManager& hashMap) {
    SearchCriteria criteria;
    criteria.verifiedOnly = true;
    return search(hashMap, criteria);
}

// Search integrity valid evidence only
vector<Evidence> AdvancedSearch::searchIntegrityValidOnly(const HashMapManager& hashMap) {
    SearchCriteria criteria;
    criteria.integrityValidOnly = true;
    return search(hashMap, criteria);
}

// Display search results in table format
void AdvancedSearch::displayResults(const vector<Evidence>& results) {
    if (results.empty()) {
        cout << "\n[!] No evidence found matching criteria.\n";
        return;
    }

    cout << "\n" << string(120, '=') << "\n";
    cout << "[SEARCH RESULTS] Found " << results.size() << " evidence item(s)\n";
    cout << string(120, '=') << "\n";

    cout << left << setw(14) << "Evidence ID" << setw(12) << "Case ID"
        << setw(20) << "Description" << setw(16) << "Hash"
        << setw(12) << "Verified" << setw(12) << "Integrity"
        << setw(10) << "Priority" << "Timestamp" << endl;
    cout << string(120, '=') << endl;

    for (const auto& e : results) {
        e.display();
    }

    cout << string(120, '=') << "\n";
}

// Export search results to text file
void AdvancedSearch::exportResultsSummary(const vector<Evidence>& results, const string& filename) {
    ofstream outFile(filename);
    if (!outFile) {
        cout << "[ERROR] Cannot create export file: " << filename << "\n";
        return;
    }

    time_t now = time(nullptr);
    char timeBuf[26];
    ctime_s(timeBuf, sizeof(timeBuf), &now);

    outFile << "========================================\n";
    outFile << "EVIDENCE SEARCH RESULTS REPORT\n";
    outFile << "========================================\n";
    outFile << "Generated: " << timeBuf;
    outFile << "Total Results: " << results.size() << "\n";
    outFile << "========================================\n\n";

    for (size_t i = 0; i < results.size(); i++) {
        const Evidence& e = results[i];

        outFile << "Result #" << (i + 1) << "\n";
        outFile << string(40, '-') << "\n";
        outFile << "Evidence ID:     " << e.id << "\n";
        outFile << "Case ID:         " << e.caseID << "\n";
        outFile << "Description:     " << e.description << "\n";
        outFile << "Hash:            " << (e.hash.empty() ? "Not calculated" : e.hash) << "\n";
        outFile << "Priority:        " << static_cast<int>(e.priority) << "\n";
        outFile << "Verified:        " << (e.verified ? "YES" : "NO") << "\n";
        outFile << "Integrity:       " << (e.integrityValid ? "VALID" : "UNCHECKED") << "\n";
        outFile << "File Size:       " << e.fileSize << " bytes\n";
        outFile << "Collector:       " << e.metadata.collectorName << "\n";
        outFile << "Location:        " << e.metadata.location << "\n";
        outFile << "Source Type:     " << e.metadata.sourceType << "\n";

        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&e.timestamp));
        outFile << "Timestamp:       " << timestamp << "\n";
        outFile << "\n";
    }

    outFile << "========================================\n";
    outFile << "END OF REPORT\n";
    outFile << "========================================\n";

    outFile.close();
    cout << "[OK] Results exported to: " << filename << "\n";
}

// Sort by priority (high to low by default)
void AdvancedSearch::sortByPriority(vector<Evidence>& results, bool descending) {
    sort(results.begin(), results.end(), [descending](const Evidence& a, const Evidence& b) {
        return descending ? (a.priority > b.priority) : (a.priority < b.priority);
        });
}

// Sort by date (newest first by default)
void AdvancedSearch::sortByDate(vector<Evidence>& results, bool newest) {
    sort(results.begin(), results.end(), [newest](const Evidence& a, const Evidence& b) {
        return newest ? (a.timestamp > b.timestamp) : (a.timestamp < b.timestamp);
        });
}

// Sort by ID (alphabetical)
void AdvancedSearch::sortByID(vector<Evidence>& results) {
    sort(results.begin(), results.end(), [](const Evidence& a, const Evidence& b) {
        return a.id < b.id;
        });
}