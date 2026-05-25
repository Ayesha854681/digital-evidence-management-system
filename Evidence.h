#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <string>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <vector>

// Add this BEFORE including Windows headers to fix byte ambiguity
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

using namespace std;

enum class Priority { LOW = 1, MEDIUM = 2, HIGH = 3, CRITICAL = 4 };

struct EvidenceMetadata {
    string collectorName;       // Who collected the evidence
    string location;            // Physical / digital location
    string deviceInfo;          // Device make/model/serial
    string sourceType;          // File/Image/Video/Network/Mobile
    string acquisitionMethod;   // Direct Copy / Forensic Image / Capture
    string notes;               // Extra notes
    time_t collectionTime;      // When evidence was collected

    vector<string> custodyChain;

    EvidenceMetadata() : collectionTime(0) {}

    void addCustodyEntry(const string& person, const string& action) {
        time_t now = time(nullptr);
        char buf[25];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        custodyChain.push_back(
            string(buf) + " | " + person + " | " + action
        );
    }
};

struct Evidence {
    string id;
    string caseID;
    string description;
    string hash;
    string filePath;
    long long fileSize;
    Priority priority;
    bool verified;
    bool integrityValid;
    time_t timestamp;
    string storedFilePath;

    EvidenceMetadata metadata;

    Evidence() : id(""), caseID(""), description(""), hash(""), filePath(""),
        fileSize(0), priority(Priority::MEDIUM), verified(false),
        integrityValid(false), timestamp(0), storedFilePath("") {
        metadata.collectionTime = time(nullptr);
    }

    Evidence(string i, string c, string d, string fp, long long s, Priority p)
        : id(i), caseID(c), description(d), hash(""), filePath(fp),
        fileSize(s), priority(p), verified(false), integrityValid(false),
        timestamp(time(nullptr)), storedFilePath("") {
        metadata.collectionTime = time(nullptr);
    }

    void display() const {
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&timestamp));

        cout << left << setw(14) << id << setw(12) << caseID
            << setw(20) << description
            << setw(16) << (hash.empty() ? "NO HASH" : hash.substr(0, 12) + "...")
            << setw(12) << (verified ? "YES" : "NO")
            << setw(12) << (integrityValid ? "VALID" : "UNCHECKED")
            << setw(10) << static_cast<int>(priority)
            << buf << endl;
    }

    void displayDetailed() const {
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));

        cout << "\n" << string(80, '=') << "\n";
        cout << "Evidence ID:       " << id << "\n";
        cout << "Case ID:           " << caseID << "\n";
        cout << "Description:       " << description << "\n";
        cout << "File Path:         " << (filePath.empty() ? "Not specified" : filePath) << "\n";
        cout << "File Size:         " << fileSize << " bytes\n";
        cout << "Hash (SHA-256):    " << (hash.empty() ? "Not calculated" : hash) << "\n";
        cout << "Priority:          " << static_cast<int>(priority) << "\n";
        cout << "Verified:          " << (verified ? "YES" : "NO") << "\n";
        cout << "Integrity Status:  " << (integrityValid ? "VALID" : "UNCHECKED") << "\n";
        cout << "Timestamp:         " << buf << "\n";

        cout << "\n--- Evidence Metadata ---\n";
        cout << "Collector:      " << metadata.collectorName << "\n";
        cout << "Location:       " << metadata.location << "\n";
        cout << "Device Info:    " << metadata.deviceInfo << "\n";
        cout << "Source Type:    " << metadata.sourceType << "\n";
        cout << "Acquisition:    " << metadata.acquisitionMethod << "\n";

        cout << "\n--- Chain of Custody ---\n";
        for (const auto& entry : metadata.custodyChain) {
            cout << entry << "\n";
        }

        cout << string(80, '=') << "\n";
    }
};

struct AuditLog {
    string action;
    string evidenceID;
    string details;
    time_t timestamp;

    AuditLog(string act = "", string eID = "", string det = "")
        : action(act), evidenceID(eID), details(det), timestamp(time(nullptr)) {
    }

    void display() const {
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&timestamp));
        cout << left << setw(16) << buf << setw(12) << action
            << setw(14) << evidenceID << setw(30) << details << endl;
    }
};