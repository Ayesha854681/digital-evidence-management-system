#pragma once
#include "Evidence.h"
#include <vector>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <iomanip>

using namespace std;

struct SearchCriteria {
    string evidenceID;
    string caseID;
    string keyword;           // Search in description
    Priority minPriority;
    Priority maxPriority;
    bool verifiedOnly;
    bool integrityValidOnly;
    time_t startDate;
    time_t endDate;
    string collectorName;
    string sourceType;

    // Constructor with defaults
    SearchCriteria()
        : evidenceID(""),
        caseID(""),
        keyword(""),
        minPriority(Priority::LOW),
        maxPriority(Priority::CRITICAL),
        verifiedOnly(false),
        integrityValidOnly(false),
        startDate(0),
        endDate(0),
        collectorName(""),
        sourceType("") {}

    // Check if any criteria is set
    bool isEmpty() const {
        return evidenceID.empty() &&
            caseID.empty() &&
            keyword.empty() &&
            collectorName.empty() &&
            sourceType.empty() &&
            !verifiedOnly &&
            !integrityValidOnly &&
            startDate == 0 &&
            endDate == 0 &&
            minPriority == Priority::LOW &&
            maxPriority == Priority::CRITICAL;
    }

    // Display current criteria
    void display() const {
        cout << "\n========== Search Criteria ==========\n";
        if (!evidenceID.empty())
            cout << "Evidence ID: " << evidenceID << "\n";
        if (!caseID.empty())
            cout << "Case ID: " << caseID << "\n";
        if (!keyword.empty())
            cout << "Keyword: " << keyword << "\n";
        if (!collectorName.empty())
            cout << "Collector: " << collectorName << "\n";
        if (!sourceType.empty())
            cout << "Source Type: " << sourceType << "\n";
        if (verifiedOnly)
            cout << "Status: Verified Only\n";
        if (integrityValidOnly)
            cout << "Integrity: Valid Only\n";
        if (minPriority != Priority::LOW || maxPriority != Priority::CRITICAL) {
            cout << "Priority Range: " << static_cast<int>(minPriority)
                << " to " << static_cast<int>(maxPriority) << "\n";
        }
        if (startDate > 0 || endDate > 0) {
            cout << "Date Range: ";
            if (startDate > 0) {
                char buf[20];
                strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&startDate));
                cout << buf;
            }
            else {
                cout << "Any";
            }
            cout << " to ";
            if (endDate > 0) {
                char buf[20];
                strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&endDate));
                cout << buf;
            }
            else {
                cout << "Any";
            }
            cout << "\n";
        }
        cout << "=====================================\n";
    }
};