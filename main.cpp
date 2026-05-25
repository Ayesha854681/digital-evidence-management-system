#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include "HashMapManager.h"
#include "PriorityQueueManager.h"
#include "AuditStackManager.h"
#include "IDGenerator.h"
#include "AdvancedSearch.h"
#include "SearchCriteria.h"
#include "BackupManager.h"
#include "AccessControl.h"
#include "InputValidator.h"
using namespace std;

HashMapManager hashMap;
PriorityQueueManager priorityQueue;
AuditStackManager auditStack;
BackupManager backupManager;
AccessControlManager accessControl;

// Helper function to clear input buffer properly
void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Helper function to get validated string input
string getValidatedStringInput(const string& prompt,
    bool (*validatorFunc)(const string&) = nullptr,
    const string& errorMsg = "Invalid input!",
    bool allowEmpty = false) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);

        // Trim whitespace
        size_t start = input.find_first_not_of(" \t\n\r");
        size_t end = input.find_last_not_of(" \t\n\r");
        if (start != string::npos) {
            input = input.substr(start, end - start + 1);
        }
        else {
            input = "";
        }

        if (input.empty() && !allowEmpty) {
            cout << "[ERROR] Input cannot be empty. Please try again.\n";
            continue;
        }

        if (allowEmpty && input.empty()) {
            return input;
        }

        if (validatorFunc == nullptr || validatorFunc(input)) {
            return input;
        }

        cout << "[ERROR] " << errorMsg << "\n";
    }
}

// Helper function to get validated integer input
int getValidatedIntInput(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        if (InputValidator::getValidatedInt(value, min, max)) {
            return value;
        }
        cout << "[ERROR] Please enter a number between " << min << " and " << max << ".\n";
    }
}

// Helper function to get validated long long input
long long getValidatedLongLongInput(const string& prompt, long long min, long long max) {
    long long value;
    while (true) {
        cout << prompt;
        if (InputValidator::getValidatedLongLong(value, min, max)) {
            return value;
        }
        cout << "[ERROR] Please enter a number between " << min << " and " << max << ".\n";
    }
}

// Helper function to get validated hash input
string getValidatedHashInput(const string& prompt) {
    string hash;
    while (true) {
        hash = getValidatedStringInput(prompt, nullptr, "", false);
        auto result = InputValidator::validateHashWithMessage(hash);
        if (result.first) {
            return hash;
        }
        cout << "[ERROR] " << result.second << "\n";
    }
}

static void menu1() {
    int choice;
    while (true) {
        cout << "\n---Evidence Registration & Storage---\n";
        cout << "1. Register New Digital Evidence (Manual Hash)\n";
        cout << "2. Register New Digital Evidence (Auto-Calculate Hash)\n";
        cout << "3. Update Existing Evidence Information\n";
        cout << "4. Retrieve Evidence by Evidence ID\n";
        cout << "5. Remove Evidence from System\n";
        cout << "6. Display All Stored Evidence\n";
        cout << "7. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 7);

        if (choice == 1) {
            // MANUAL HASH REGISTRATION
            string desc, hash;
            long long size;
            int pri;

            string id = IDGenerator::generateEvidenceID();
            string cID = IDGenerator::generateCaseID();

            cout << "\n[GENERATED] Evidence ID: " << id << endl;
            cout << "[GENERATED] Case ID: " << cID << endl;

            desc = getValidatedStringInput(
                "Description: ",
                InputValidator::isValidDescription,
                "Description must be 1-500 characters with printable text only."
            );
            desc = InputValidator::sanitizeDescription(desc);

            hash = getValidatedHashInput("Hash (SHA-256, 64 hex characters): ");

            size = getValidatedLongLongInput(
                "File Size (bytes, max 1TB): ",
                1,
                1099511627776LL
            );

            pri = getValidatedIntInput(
                "Priority (1=LOW, 2=MEDIUM, 3=HIGH, 4=CRITICAL): ",
                1,
                4
            );

            Priority p = Priority::MEDIUM;
            if (pri == 1) p = Priority::LOW;
            else if (pri == 3) p = Priority::HIGH;
            else if (pri == 4) p = Priority::CRITICAL;

            Evidence e(id, cID, desc, hash, size, p);

            e.metadata.collectorName = getValidatedStringInput(
                "Collector Name: ",
                InputValidator::isValidName,
                "Name must contain only letters, spaces, hyphens, and apostrophes (max 100 chars)."
            );

            e.metadata.location = getValidatedStringInput(
                "Collection Location: ",
                [](const string& s) { return InputValidator::isValidTextField(s, 200); },
                "Location must be 1-200 characters."
            );
            e.metadata.location = InputValidator::sanitizeText(e.metadata.location);

            e.metadata.deviceInfo = getValidatedStringInput(
                "Device Info (Make/Model/Serial): ",
                [](const string& s) { return InputValidator::isValidTextField(s, 200); },
                "Device info must be 1-200 characters."
            );
            e.metadata.deviceInfo = InputValidator::sanitizeText(e.metadata.deviceInfo);

            e.metadata.sourceType = getValidatedStringInput(
                "Source Type (File/Image/Video/etc): ",
                [](const string& s) { return InputValidator::isValidTextField(s, 50); },
                "Source type must be 1-50 characters."
            );
            e.metadata.sourceType = InputValidator::sanitizeText(e.metadata.sourceType);

            e.metadata.acquisitionMethod = getValidatedStringInput(
                "Acquisition Method: ",
                [](const string& s) { return InputValidator::isValidTextField(s, 200); },
                "Acquisition method must be 1-200 characters."
            );
            e.metadata.acquisitionMethod = InputValidator::sanitizeText(e.metadata.acquisitionMethod);

            e.metadata.notes = getValidatedStringInput(
                "Notes (optional): ",
                [](const string& s) { return InputValidator::isValidTextField(s, 1000); },
                "Notes must be under 1000 characters.",
                true
            );
            if (!e.metadata.notes.empty()) {
                e.metadata.notes = InputValidator::sanitizeText(e.metadata.notes);
            }

            e.metadata.addCustodyEntry(
                e.metadata.collectorName,
                "Evidence collected (manual hash)"
            );
            hashMap.registerEvidence(e);
            auditStack.logAction("REGISTER", id, "New evidence registered (manual hash)");
            cout << "\n[✓] Evidence registered successfully!\n";
        }
        else if (choice == 2) {
            // AUTO-CALCULATE HASH
            string desc, filepath;
            int pri;

            string id = IDGenerator::generateEvidenceID();
            string cID = IDGenerator::generateCaseID();

            cout << "\n[GENERATED] Evidence ID: " << id << endl;
            cout << "[GENERATED] Case ID: " << cID << endl;

            desc = getValidatedStringInput(
                "Description: ",
                InputValidator::isValidDescription,
                "Description must be 1-500 characters with printable text only."
            );
            desc = InputValidator::sanitizeDescription(desc);

            filepath = getValidatedStringInput(
                "File Path (e.g., C:\\evidence\\file.pdf): ",
                InputValidator::isValidFilePath,
                "Invalid file path. Must be under 260 chars and not contain <>|\"*?"
            );
            filepath = InputValidator::sanitizeFilePath(filepath);

            pri = getValidatedIntInput(
                "Priority (1=LOW, 2=MEDIUM, 3=HIGH, 4=CRITICAL): ",
                1,
                4
            );

            Priority p = Priority::MEDIUM;
            if (pri == 1) p = Priority::LOW;
            else if (pri == 3) p = Priority::HIGH;
            else if (pri == 4) p = Priority::CRITICAL;

            Evidence e(id, cID, desc, filepath, 0, p);

            e.metadata.collectorName = getValidatedStringInput(
                "Collector Name: ",
                InputValidator::isValidName,
                "Name must contain only letters, spaces, hyphens, and apostrophes (max 100 chars)."
            );

            e.metadata.location = getValidatedStringInput(
                "Collection Location: ",
                [](const string& s) { return InputValidator::isValidTextField(s, 200); },
                "Location must be 1-200 characters."
            );
            e.metadata.location = InputValidator::sanitizeText(e.metadata.location);

            e.metadata.deviceInfo = getValidatedStringInput(
                "Device Info: ",
                [](const string& s) { return InputValidator::isValidTextField(s, 200); },
                "Device info must be 1-200 characters."
            );
            e.metadata.deviceInfo = InputValidator::sanitizeText(e.metadata.deviceInfo);

            e.metadata.sourceType = getValidatedStringInput(
                "Source Type: ",
                [](const string& s) { return InputValidator::isValidTextField(s, 50); },
                "Source type must be 1-50 characters."
            );
            e.metadata.sourceType = InputValidator::sanitizeText(e.metadata.sourceType);

            e.metadata.acquisitionMethod = getValidatedStringInput(
                "Acquisition Method: ",
                [](const string& s) { return InputValidator::isValidTextField(s, 200); },
                "Acquisition method must be 1-200 characters."
            );
            e.metadata.acquisitionMethod = InputValidator::sanitizeText(e.metadata.acquisitionMethod);

            e.metadata.notes = getValidatedStringInput(
                "Notes (optional): ",
                [](const string& s) { return InputValidator::isValidTextField(s, 1000); },
                "Notes must be under 1000 characters.",
                true
            );
            if (!e.metadata.notes.empty()) {
                e.metadata.notes = InputValidator::sanitizeText(e.metadata.notes);
            }

            e.metadata.addCustodyEntry(
                e.metadata.collectorName,
                "Evidence collected (auto hash)"
            );

            hashMap.registerEvidenceWithFile(e);
            auditStack.logAction("REGISTER", id, "New evidence registered with auto-hash");
            cout << "\n[✓] Evidence registered successfully with auto-calculated hash!\n";
        }
        else if (choice == 3) {
            // UPDATE EVIDENCE
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001 (E + 8 digits)"
            );

            Evidence* e = hashMap.retrieveEvidence(id);
            if (e) {
                cout << "\n[CURRENT] Description: " << e->description << "\n";
                cout << "[CURRENT] Hash: " << e->hash << "\n\n";

                string desc = getValidatedStringInput(
                    "New Description: ",
                    InputValidator::isValidDescription,
                    "Description must be 1-500 characters."
                );
                desc = InputValidator::sanitizeDescription(desc);

                string hash = getValidatedHashInput("New Hash: ");

                e->description = desc;
                e->hash = hash;
                hashMap.updateEvidence(id, *e);
                auditStack.logAction("UPDATE", id, "Evidence updated");
                cout << "\n[✓] Evidence updated successfully!\n";
            }
            else {
                cout << "\n[ERROR] Evidence not found!\n";
            }
        }
        else if (choice == 4) {
            // RETRIEVE EVIDENCE
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );

            Evidence* e = hashMap.retrieveEvidence(id);
            if (e) {
                e->displayDetailed();
            }
            else {
                cout << "\n[ERROR] Evidence not found!\n";
            }
        }
        else if (choice == 5) {
            // REMOVE EVIDENCE
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );

            cout << "\n[WARNING] Are you sure you want to delete this evidence? (yes/no): ";
            string confirm;
            getline(cin, confirm);

            if (confirm == "yes" || confirm == "YES") {
                hashMap.removeEvidence(id);
                auditStack.logAction("DELETE", id, "Evidence deleted");
                cout << "\n[✓] Evidence deleted successfully!\n";
            }
            else {
                cout << "\n[CANCELLED] Deletion cancelled.\n";
            }
        }
        else if (choice == 6) {
            hashMap.displayAll();
        }
        else if (choice == 7) {
            return;
        }
    }
}

static void menu2() {
    int choice;
    while (true) {
        cout << "\n---Evidence Integrity Verification---\n";
        cout << "1. Verify Evidence Integrity (Mark as Verified)\n";
        cout << "2. Verify File Integrity (Check Actual File Hash)\n";
        cout << "3. View Evidence Verification Status\n";
        cout << "4. Recalculate Hash from File\n";
        cout << "5. Update Hash Manually\n";
        cout << "6. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 6);

        if (choice == 1) {
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );
            hashMap.verifyEvidence(id);
            auditStack.logAction("VERIFY", id, "Evidence marked as verified");
        }
        else if (choice == 2) {
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );
            bool isValid = hashMap.verifyFileIntegrity(id);

            if (isValid) {
                auditStack.logAction("INTEGRITY_CHECK", id, "File integrity VALID");
            }
            else {
                auditStack.logAction("INTEGRITY_VIOLATION", id, "File integrity FAILED");
            }
        }
        else if (choice == 3) {
            cout << "\n========================================\n";
            cout << "    VERIFICATION STATUS REPORT          \n";
            cout << "========================================\n";
            cout << " Verified:          " << setw(19) << hashMap.getVerifiedCount() << "\n";
            cout << " Unverified:        " << setw(19) << hashMap.getUnverifiedCount() << "\n";
            cout << " Integrity Valid:   " << setw(19) << hashMap.getIntegrityValidCount() << "\n";
            cout << " Integrity Issues:  " << setw(19) << hashMap.getIntegrityInvalidCount() << "\n";
            cout << "========================================\n";
        }
        else if (choice == 4) {
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );
            hashMap.recalculateHash(id);
            auditStack.logAction("HASH_RECALC", id, "Hash recalculated from file");
        }
        else if (choice == 5) {
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );

            string newHash = getValidatedHashInput("New Hash: ");

            hashMap.updateHash(id, newHash);
            auditStack.logAction("HASH_UPDATE", id, "Hash manually updated");
        }
        else if (choice == 6) {
            return;
        }
    }
}

static void menu3() {
    int choice;
    while (true) {
        cout << "\n---Case Priority Handling---\n";
        cout << "1. Assign/Update Evidence Priority\n";
        cout << "2. View Highest Priority Evidence\n";
        cout << "3. Process Next Priority Case\n";
        cout << "4. Display All Priority-Based Evidence\n";
        cout << "5. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 5);

        if (choice == 1) {
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );

            int pri = getValidatedIntInput(
                "Priority (1=LOW, 2=MEDIUM, 3=HIGH, 4=CRITICAL): ",
                1,
                4
            );

            Priority p = Priority::MEDIUM;
            if (pri == 1) p = Priority::LOW;
            else if (pri == 3) p = Priority::HIGH;
            else if (pri == 4) p = Priority::CRITICAL;

            Evidence* e = hashMap.retrieveEvidence(id);
            if (e) {
                e->priority = p;
                priorityQueue.assignPriority(*e);
                auditStack.logAction("PRIORITY", id, "Priority assigned");
                cout << "\n[✓] Priority assigned successfully!\n";
            }
            else {
                cout << "\n[ERROR] Evidence not found!\n";
            }
        }
        else if (choice == 2) {
            Evidence e = priorityQueue.getHighestPriority();
            if (!e.id.empty()) {
                cout << "\nHighest Priority Case:\n";
                e.displayDetailed();
            }
            else {
                cout << "\n[INFO] No cases in queue!\n";
            }
        }
        else if (choice == 3) {
            priorityQueue.processNextCase();
            auditStack.logAction("PROCESS", "", "Case processed");
        }
        else if (choice == 4) {
            priorityQueue.displayAll();
        }
        else if (choice == 5) {
            return;
        }
    }
}

static void menu4() {
    int choice;
    while (true) {
        cout << "\n---Audit Trail & Chain of Custody---\n";
        cout << "1. View Most Recent Evidence Action\n";
        cout << "2. View Complete Audit History\n";
        cout << "3. Undo Last Action (Rollback)\n";
        cout << "4. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 4);

        if (choice == 1) {
            AuditLog log = auditStack.viewMostRecent();
            if (!log.action.empty()) {
                cout << "\nMost Recent Action:\n";
                log.display();
            }
            else {
                cout << "\n[INFO] No audit history!\n";
            }
        }
        else if (choice == 2) {
            auditStack.viewComplete();
        }
        else if (choice == 3) {
            auditStack.undoLastAction();
        }
        else if (choice == 4) {
            return;
        }
    }
}

static void menu5() {
    int choice;
    while (true) {
        cout << "\n---Evidence Search & Reports---\n";
        cout << "1. Search Evidence by Evidence ID\n";
        cout << "2. Generate Evidence Summary Report\n";
        cout << "3. Generate Priority-Based Case Report\n";
        cout << "4. Generate Verification Status Report\n";
        cout << "5. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 5);

        if (choice == 1) {
            string id = getValidatedStringInput(
                "Evidence ID: ",
                InputValidator::isValidEvidenceID,
                "Evidence ID must be in format E20240001"
            );

            Evidence* e = hashMap.retrieveEvidence(id);
            if (e) {
                e->displayDetailed();
            }
            else {
                cout << "\n[ERROR] Evidence not found!\n";
            }
        }
        else if (choice == 2) {
            hashMap.displayStats();
        }
        else if (choice == 3) {
            priorityQueue.displayStats();
        }
        else if (choice == 4) {
            cout << "\n========================================\n";
            cout << "    VERIFICATION STATUS REPORT          \n";
            cout << "========================================\n";
            cout << " Verified:          " << setw(19) << hashMap.getVerifiedCount() << "\n";
            cout << " Unverified:        " << setw(19) << hashMap.getUnverifiedCount() << "\n";
            cout << " Integrity Valid:   " << setw(19) << hashMap.getIntegrityValidCount() << "\n";
            cout << " Integrity Issues:  " << setw(19) << hashMap.getIntegrityInvalidCount() << "\n";
            cout << "========================================\n";
        }
        else if (choice == 5) {
            return;
        }
    }
}

static void menu6() {
    int choice;
    while (true) {
        cout << "\n---Administrative Controls---\n";
        cout << "1. View System Activity Log\n";
        cout << "2. Clear Audit Trail\n";
        cout << "3. Reset Evidence Database\n";
        cout << "4. System Statistics Overview\n";
        cout << "5. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 5);

        if (choice == 1) {
            auditStack.viewComplete();
        }
        else if (choice == 2) {
            cout << "\n[WARNING] This will clear ALL audit logs. Are you sure? (yes/no): ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "yes" || confirm == "YES") {
                auditStack.clearAll();
                cout << "\n[✓] Audit trail cleared!\n";
            }
            else {
                cout << "\n[CANCELLED] Operation cancelled.\n";
            }
        }
        else if (choice == 3) {
            cout << "\n[DANGER] This will DELETE ALL EVIDENCE from the database!\n";
            cout << "This action CANNOT be undone. Are you absolutely sure? (yes/no): ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "yes" || confirm == "YES") {
                hashMap.clearAll();
                cout << "\n[✓] Database reset complete!\n";
            }
            else {
                cout << "\n[CANCELLED] Operation cancelled.\n";
            }
        }
        else if (choice == 4) {
            cout << "\n========================================\n";
            cout << "      SYSTEM STATISTICS OVERVIEW        \n";
            cout << "========================================\n";
            hashMap.displayStats();
            priorityQueue.displayStats();
            auditStack.displayStats();
        }
        else if (choice == 5) {
            return;
        }
    }
}

static void menuAdvancedSearch() {
    int choice;
    while (true) {
        cout << "\n---Advanced Search & Filter---\n";
        cout << "1. Quick Search by Keyword\n";
        cout << "2. Search by Priority Level\n";
        cout << "3. Search by Collector Name\n";
        cout << "4. Search Verified Evidence Only\n";
        cout << "5. Search Integrity Valid Evidence Only\n";
        cout << "6. Advanced Multi-Criteria Search\n";
        cout << "7. Search by Date Range\n";
        cout << "8. Return to Main Menu\n";

        choice = getValidatedIntInput("Choice: ", 1, 8);

        if (choice == 1) {
            // Quick keyword search
            string keyword = getValidatedStringInput(
                "Enter keyword (searches in description): ",
                [](const string& s) { return !s.empty() && s.length() <= 100; },
                "Keyword must be 1-100 characters.",
                false
            );

            cout << "\nSearching for: \"" << keyword << "\"...\n";
            vector<Evidence> results = AdvancedSearch::searchByKeyword(hashMap, keyword);
            AdvancedSearch::displayResults(results);

            if (!results.empty()) {
                cout << "\nExport results? (y/n): ";
                char exportChoice;
                cin >> exportChoice;
                clearInputBuffer();
                if (exportChoice == 'y' || exportChoice == 'Y') {
                    AdvancedSearch::exportResultsSummary(results, "keyword_search_results.txt");
                }
            }
        }
        else if (choice == 2) {
            // Search by priority
            int pri = getValidatedIntInput(
                "Priority (1=LOW, 2=MEDIUM, 3=HIGH, 4=CRITICAL): ",
                1,
                4
            );

            Priority p = static_cast<Priority>(pri);

            cout << "\nSearching for Priority " << pri << " evidence...\n";
            vector<Evidence> results = AdvancedSearch::searchByPriority(hashMap, p);
            AdvancedSearch::sortByDate(results, true);
            AdvancedSearch::displayResults(results);
        }
        else if (choice == 3) {
            // Search by collector
            string collector = getValidatedStringInput(
                "Collector Name (case-insensitive): ",
                InputValidator::isValidName,
                "Name must contain only letters, spaces, hyphens, and apostrophes."
            );

            cout << "\nSearching for collector: \"" << collector << "\"...\n";
            vector<Evidence> results = AdvancedSearch::searchByCollector(hashMap, collector);
            AdvancedSearch::displayResults(results);
        }
        else if (choice == 4) {
            cout << "\nSearching for verified evidence only...\n";
            vector<Evidence> results = AdvancedSearch::searchVerifiedOnly(hashMap);
            AdvancedSearch::displayResults(results);
        }
        else if (choice == 5) {
            cout << "\nSearching for integrity valid evidence only...\n";
            vector<Evidence> results = AdvancedSearch::searchIntegrityValidOnly(hashMap);
            AdvancedSearch::displayResults(results);
        }
        else if (choice == 6) {
            // Advanced multi-criteria search
            SearchCriteria criteria;

            cout << "\n========== Build Search Query ==========\n";
            cout << "Leave blank to skip any field\n\n";

            string input = getValidatedStringInput(
                "Evidence ID (exact match, or blank): ",
                nullptr, "", true
            );
            if (!input.empty()) {
                if (InputValidator::isValidEvidenceID(input)) {
                    criteria.evidenceID = input;
                }
                else {
                    cout << "[WARNING] Invalid Evidence ID format. Skipping this field.\n";
                }
            }

            input = getValidatedStringInput(
                "Case ID (exact match, or blank): ",
                nullptr, "", true
            );
            if (!input.empty()) {
                if (InputValidator::isValidCaseID(input)) {
                    criteria.caseID = input;
                }
                else {
                    cout << "[WARNING] Invalid Case ID format. Skipping this field.\n";
                }
            }

            input = getValidatedStringInput(
                "Keyword in description: ",
                nullptr, "", true
            );
            if (!input.empty()) criteria.keyword = input;

            input = getValidatedStringInput(
                "Collector Name: ",
                nullptr, "", true
            );
            if (!input.empty()) criteria.collectorName = input;

            input = getValidatedStringInput(
                "Source Type (File/Image/Video/etc): ",
                nullptr, "", true
            );
            if (!input.empty()) criteria.sourceType = input;

            cout << "Minimum Priority (1-4, 0 to skip): ";
            int minPri;
            cin >> minPri;
            if (minPri >= 1 && minPri <= 4) {
                criteria.minPriority = static_cast<Priority>(minPri);
            }

            cout << "Maximum Priority (1-4, 0 to skip): ";
            int maxPri;
            cin >> maxPri;
            if (maxPri >= 1 && maxPri <= 4) {
                criteria.maxPriority = static_cast<Priority>(maxPri);
            }
            clearInputBuffer();

            cout << "Verified Only? (y/n): ";
            char verified;
            cin >> verified;
            criteria.verifiedOnly = (verified == 'y' || verified == 'Y');

            cout << "Integrity Valid Only? (y/n): ";
            char integrity;
            cin >> integrity;
            criteria.integrityValidOnly = (integrity == 'y' || integrity == 'Y');
            clearInputBuffer();

            criteria.display();

            cout << "\nExecuting search...\n";
            vector<Evidence> results = AdvancedSearch::search(hashMap, criteria);

            if (!results.empty()) {
                int sortChoice = getValidatedIntInput(
                    "\nSort results by:\n1. Priority (High to Low)\n2. Date (Newest First)\n3. Evidence ID\n0. No sorting\nChoice: ",
                    0,
                    3
                );

                if (sortChoice == 1) {
                    AdvancedSearch::sortByPriority(results, true);
                    cout << "[Sorted by Priority]\n";
                }
                else if (sortChoice == 2) {
                    AdvancedSearch::sortByDate(results, true);
                    cout << "[Sorted by Date - Newest First]\n";
                }
                else if (sortChoice == 3) {
                    AdvancedSearch::sortByID(results);
                    cout << "[Sorted by Evidence ID]\n";
                }
            }

            AdvancedSearch::displayResults(results);

            if (!results.empty()) {
                cout << "\nExport results? (y/n): ";
                char exportChoice;
                cin >> exportChoice;
                clearInputBuffer();
                if (exportChoice == 'y' || exportChoice == 'Y') {
                    AdvancedSearch::exportResultsSummary(results, "advanced_search_results.txt");
                }
            }
        }
        else if (choice == 7) {
            // Date range search
            SearchCriteria criteria;

            cout << "\n========== Date Range Search ==========\n";

            string startStr = getValidatedStringInput(
                "Start Date (YYYY-MM-DD, or blank for any): ",
                nullptr, "", true
            );

            if (!startStr.empty()) {
                if (InputValidator::isValidDateFormat(startStr)) {
                    struct tm tm = {};
                    if (sscanf(startStr.c_str(), "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3) {
                        tm.tm_year -= 1900;
                        tm.tm_mon -= 1;
                        tm.tm_hour = 0;
                        tm.tm_min = 0;
                        tm.tm_sec = 0;
                        criteria.startDate = mktime(&tm);
                    }
                }
                else {
                    cout << "[WARNING] Invalid date format. Skipping start date.\n";
                }
            }

            string endStr = getValidatedStringInput(
                "End Date (YYYY-MM-DD, or blank for any): ",
                nullptr, "", true
            );

            if (!endStr.empty()) {
                if (InputValidator::isValidDateFormat(endStr)) {
                    struct tm tm = {};
                    if (sscanf(endStr.c_str(), "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3) {
                        tm.tm_year -= 1900;
                        tm.tm_mon -= 1;
                        tm.tm_hour = 23;
                        tm.tm_min = 59;
                        tm.tm_sec = 59;
                        criteria.endDate = mktime(&tm);
                    }
                }
                else {
                    cout << "[WARNING] Invalid date format. Skipping end date.\n";
                }
            }

            criteria.display();

            cout << "\nExecuting date range search...\n";
            vector<Evidence> results = AdvancedSearch::search(hashMap, criteria);
            AdvancedSearch::sortByDate(results, true);
            AdvancedSearch::displayResults(results);
        }
        else if (choice == 8) {
            return;
        }
    }
}

static void menu7() {
    int choice;
    while (true) {
        backupManager.displayBackupMenu();

        choice = getValidatedIntInput("Choice: ", 1, 5);

        if (choice == 1) {
            backupManager.createBackup();
            auditStack.logAction("BACKUP", "", "System backup created");
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 2) {
            vector<string> backups = backupManager.listBackups();
            backupManager.displayBackupsList(backups);
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 3) {
            vector<string> backups = backupManager.listBackups();

            if (backups.empty()) {
                cout << "\n[ERROR] No backups available to restore.\n";
                cout << "\nPress Enter to continue...";
                cin.get();
                continue;
            }

            backupManager.displayBackupsList(backups);

            int backupChoice = getValidatedIntInput(
                "\nEnter backup number to restore (0 to cancel): ",
                0,
                (int)backups.size()
            );

            if (backupChoice == 0) {
                cout << "\n[CANCELLED] Restore cancelled.\n";
            }
            else {
                if (backupManager.restoreBackup(backups[backupChoice - 1])) {
                    auditStack.logAction("RESTORE", "", "System backup restored: " + backups[backupChoice - 1]);
                }
            }

            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 4) {
            vector<string> backups = backupManager.listBackups();

            if (backups.empty()) {
                cout << "\n[ERROR] No backups available to delete.\n";
                cout << "\nPress Enter to continue...";
                cin.get();
                continue;
            }

            backupManager.displayBackupsList(backups);

            int backupChoice = getValidatedIntInput(
                "\nEnter backup number to delete (0 to cancel): ",
                0,
                (int)backups.size()
            );

            if (backupChoice == 0) {
                cout << "\n[CANCELLED] Delete cancelled.\n";
            }
            else {
                if (backupManager.deleteBackup(backups[backupChoice - 1])) {
                    auditStack.logAction("DELETE_BACKUP", "", "Backup deleted: " + backups[backupChoice - 1]);
                }
            }

            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 5) {
            return;
        }
    }
}

int main() {
    IDGenerator::loadCounters();

    cout << "\n=================================================================\n";
    cout << "        DIGITAL EVIDENCE VERIFICATION SYSTEM v2.0\n";
    cout << "            (With Enhanced Input Validation)\n";
    cout << "=================================================================\n";

    int choice;
    while (true) {
        cout << "\n=================================================================\n";
        cout << "        DIGITAL EVIDENCE VERIFICATION SYSTEM\n";
        cout << "            (Cyber Crime Unit - CLI)\n";
        cout << "           NOW WITH SHA-256 INTEGRITY VERIFICATION\n";
        cout << "=================================================================\n\n";
        cout << "1. Evidence Registration & Storage\n";
        cout << "2. Evidence Integrity Verification\n";
        cout << "3. Case Priority Handling\n";
        cout << "4. Audit Trail & Chain of Custody\n";
        cout << "5. Evidence Search & Reports\n";
        cout << "6. Administrative Controls\n";
        cout << "7. Advanced Search & Filter\n";
        cout << "8. Backup & Recovery\n";
        cout << "9. Exit System\n";

        choice = getValidatedIntInput("Enter your choice: ", 1, 9);

        if (choice == 1) menu1();
        else if (choice == 2) menu2();
        else if (choice == 3) menu3();
        else if (choice == 4) menu4();
        else if (choice == 5) menu5();
        else if (choice == 6) menu6();
        else if (choice == 7) menuAdvancedSearch();
        else if (choice == 8) menu7();
        else if (choice == 9) {
            cout << "\n========================================\n";
            cout << "  Thank you for using the Digital Evidence System!\n";
            cout << "========================================\n";
            IDGenerator::saveCounters();
            break;
        }
    }
    return 0;
}