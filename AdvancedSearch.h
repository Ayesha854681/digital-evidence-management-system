#pragma once
#include "Evidence.h"
#include "SearchCriteria.h"
#include "HashMapManager.h"
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>

using namespace std;

class AdvancedSearch {
private:
    // Convert string to lowercase for case-insensitive search
    static string toLower(string str);

    // Check if a string contains a substring (case-insensitive)
    static bool containsIgnoreCase(const string& str, const string& substr);

    // Check if evidence matches all criteria
    static bool matchesCriteria(const Evidence& e, const SearchCriteria& c);

public:
    // Main search function
    static vector<Evidence> search(const HashMapManager& hashMap, const SearchCriteria& criteria);

    // Convenience search functions
    static vector<Evidence> searchByKeyword(const HashMapManager& hashMap, const string& keyword);
    static vector<Evidence> searchByPriority(const HashMapManager& hashMap, Priority priority);
    static vector<Evidence> searchByCollector(const HashMapManager& hashMap, const string& collector);
    static vector<Evidence> searchVerifiedOnly(const HashMapManager& hashMap);
    static vector<Evidence> searchIntegrityValidOnly(const HashMapManager& hashMap);

    // Display and export functions
    static void displayResults(const vector<Evidence>& results);
    static void exportResultsSummary(const vector<Evidence>& results, const string& filename);

    // Sorting functions
    static void sortByPriority(vector<Evidence>& results, bool descending = true);
    static void sortByDate(vector<Evidence>& results, bool newest = true);
    static void sortByID(vector<Evidence>& results);
};