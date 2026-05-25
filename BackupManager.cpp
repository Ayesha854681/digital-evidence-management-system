#define _CRT_SECURE_NO_WARNINGS
#include "BackupManager.h"
#include <iomanip>
#include <algorithm>
#include <sstream>

BackupManager::BackupManager() {
    backupDirectory = "backups/";
    createDirectory(backupDirectory);
}

// Helper: Create directory
bool BackupManager::createDirectory(const string& path) {
    if (directoryExists(path)) {
        return true;
    }

    // Create directory using Windows API
    if (_mkdir(path.c_str()) == 0) {
        return true;
    }

    // If failed, might be nested directories - try creating parent first
    size_t pos = path.find_last_of("/\\");
    if (pos != string::npos) {
        string parent = path.substr(0, pos);
        if (createDirectory(parent)) {
            return _mkdir(path.c_str()) == 0;
        }
    }

    return false;
}

// Helper: Check if directory exists
bool BackupManager::directoryExists(const string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

// Helper: Check if file exists
bool BackupManager::fileExists(const string& path) {
    return _access(path.c_str(), 0) == 0;
}

// Helper: Copy single file
bool BackupManager::copyFile(const string& source, const string& destination) {
    if (!fileExists(source)) {
        return false;
    }

    ifstream src(source, ios::binary);
    if (!src) {
        cerr << "Error: Cannot open source file: " << source << endl;
        return false;
    }

    ofstream dst(destination, ios::binary);
    if (!dst) {
        cerr << "Error: Cannot create destination file: " << destination << endl;
        return false;
    }

    dst << src.rdbuf();

    src.close();
    dst.close();

    return true;
}

// Helper: Copy entire directory
bool BackupManager::copyDirectory(const string& source, const string& destination) {
    if (!directoryExists(source)) {
        return false;
    }

    // Create destination directory
    if (!createDirectory(destination)) {
        return false;
    }

    // Use Windows API to list files
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((source + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        string filename = findData.cFileName;

        // Skip . and ..
        if (filename == "." || filename == "..") {
            continue;
        }

        string srcPath = source + "\\" + filename;
        string dstPath = destination + "\\" + filename;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively copy subdirectory
            copyDirectory(srcPath, dstPath);
        }
        else {
            // Copy file
            copyFile(srcPath, dstPath);
        }

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return true;
}

// Helper: Get list of directories
vector<string> BackupManager::getDirectoryList(const string& path) {
    vector<string> directories;

    if (!directoryExists(path)) {
        return directories;
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return directories;
    }

    do {
        string dirname = findData.cFileName;

        // Skip . and ..
        if (dirname == "." || dirname == "..") {
            continue;
        }

        // Only add directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            directories.push_back(dirname);
        }

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);

    // Sort by name (newest first due to timestamp format)
    sort(directories.rbegin(), directories.rend());

    return directories;
}

// Helper: Delete directory recursively
void BackupManager::deleteDirectory(const string& path) {
    if (!directoryExists(path)) {
        return;
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        string filename = findData.cFileName;

        if (filename == "." || filename == "..") {
            continue;
        }

        string fullPath = path + "\\" + filename;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            deleteDirectory(fullPath);
        }
        else {
            DeleteFileA(fullPath.c_str());
        }

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    RemoveDirectoryA(path.c_str());
}

// Create backup with timestamp
bool BackupManager::createBackup() {
    try {
        // Generate timestamp
        time_t now = time(nullptr);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&now));

        string backupFolder = backupDirectory + "backup_" + string(timestamp) + "\\";

        cout << "\n";
        cout << "============================================================\n";
        cout << "               CREATING BACKUP                              \n";
        cout << "============================================================\n";
        cout << "\nBackup Name: backup_" << timestamp << "\n\n";

        int filesBackedUp = 0;
        int totalFiles = 4; // Database, audit, counters, evidence folder

        // Create backup folder
        if (!createDirectory(backupFolder)) {
            cout << "[ERROR] Failed to create backup folder!\n";
            return false;
        }
        cout << "[OK] Backup folder created\n";

        // Progress indicators
        cout << "\nBacking up files:\n";
        cout << "------------------------------------------------------------\n";

        // Backup database file
        if (fileExists("evidence_database.txt")) {
            cout << "   [1/4] Copying evidence database... ";
            if (copyFile("evidence_database.txt", backupFolder + "evidence_database.txt")) {
                cout << "[OK]\n";
                filesBackedUp++;
            }
            else {
                cout << "[FAILED]\n";
            }
        }
        else {
            cout << "   Evidence database... [NOT FOUND - SKIPPED]\n";
        }

        // Backup audit trail
        if (fileExists("audit_trail.txt")) {
            cout << "   [2/4] Copying audit trail... ";
            if (copyFile("audit_trail.txt", backupFolder + "audit_trail.txt")) {
                cout << "[OK]\n";
                filesBackedUp++;
            }
            else {
                cout << "[FAILED]\n";
            }
        }
        else {
            cout << "   Audit trail... [NOT FOUND - SKIPPED]\n";
        }

        // Backup ID counters
        if (fileExists("id_counters.txt")) {
            cout << "   [3/4] Copying ID counters... ";
            if (copyFile("id_counters.txt", backupFolder + "id_counters.txt")) {
                cout << "[OK]\n";
                filesBackedUp++;
            }
            else {
                cout << "[FAILED]\n";
            }
        }
        else {
            cout << "   ID counters... [NOT FOUND - SKIPPED]\n";
        }

        // Backup evidence files folder
        if (directoryExists("evidence_files")) {
            cout << "   [4/4] Copying evidence files folder... ";
            if (copyDirectory("evidence_files", backupFolder + "evidence_files")) {
                cout << "[OK]\n";
                filesBackedUp++;
            }
            else {
                cout << "[FAILED]\n";
            }
        }
        else {
            cout << "   Evidence files folder... [NOT FOUND - SKIPPED]\n";
        }

        cout << "------------------------------------------------------------\n";

        if (filesBackedUp > 0) {
            cout << "\n";
            cout << "============================================================\n";
            cout << "             BACKUP COMPLETED SUCCESSFULLY!                 \n";
            cout << "============================================================\n";
            cout << "\nSummary:\n";
            cout << "   * Files backed up: " << filesBackedUp << "/" << totalFiles << "\n";
            cout << "   * Location: " << backupFolder << "\n";
            cout << "   * Timestamp: " << timestamp << "\n";
            cout << "\nTip: You can restore this backup anytime from the Backup menu.\n";
        }
        else {
            cout << "\n[WARNING] No files were backed up (system may be empty).\n";
        }

        return true;

    }
    catch (const exception& e) {
        cout << "\n[ERROR] Backup failed - " << e.what() << "\n";
        return false;
    }
}

// Restore from backup
bool BackupManager::restoreBackup(const string& backupName) {
    string backupPath = backupDirectory + backupName + "\\";

    if (!directoryExists(backupPath)) {
        cout << "\n[ERROR] Backup not found - " << backupName << "\n";
        return false;
    }

    cout << "\n";
    cout << "============================================================\n";
    cout << "               RESTORING BACKUP                             \n";
    cout << "============================================================\n";
    cout << "\nBackup: " << backupName << "\n";

    cout << "\n[WARNING] This will OVERWRITE your current data!\n";
    cout << "All existing evidence, audit logs, and files will be replaced.\n";
    cout << "\nAre you sure you want to continue? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore();

    if (confirm != 'y' && confirm != 'Y') {
        cout << "\n[CANCELLED] Restore operation cancelled.\n";
        return false;
    }

    try {
        int filesRestored = 0;

        cout << "\nRestoring files:\n";
        cout << "------------------------------------------------------------\n";

        // Restore database
        if (fileExists(backupPath + "evidence_database.txt")) {
            cout << "   Restoring evidence database... ";
            if (copyFile(backupPath + "evidence_database.txt", "evidence_database.txt")) {
                cout << "[OK]\n";
                filesRestored++;
            }
        }

        // Restore audit trail
        if (fileExists(backupPath + "audit_trail.txt")) {
            cout << "   Restoring audit trail... ";
            if (copyFile(backupPath + "audit_trail.txt", "audit_trail.txt")) {
                cout << "[OK]\n";
                filesRestored++;
            }
        }

        // Restore ID counters
        if (fileExists(backupPath + "id_counters.txt")) {
            cout << "   Restoring ID counters... ";
            if (copyFile(backupPath + "id_counters.txt", "id_counters.txt")) {
                cout << "[OK]\n";
                filesRestored++;
            }
        }

        // Restore evidence files
        if (directoryExists(backupPath + "evidence_files")) {
            cout << "   Restoring evidence files folder... ";
            // Remove old evidence_files if exists
            if (directoryExists("evidence_files")) {
                deleteDirectory("evidence_files");
            }
            if (copyDirectory(backupPath + "evidence_files", "evidence_files")) {
                cout << "[OK]\n";
                filesRestored++;
            }
        }

        cout << "------------------------------------------------------------\n";

        cout << "\n";
        cout << "============================================================\n";
        cout << "            RESTORE COMPLETED SUCCESSFULLY!                 \n";
        cout << "============================================================\n";
        cout << "\nSummary:\n";
        cout << "   * Files restored: " << filesRestored << "\n";
        cout << "   * Source: " << backupName << "\n";
        cout << "\n[IMPORTANT] Please restart the application to load restored data.\n";

        return true;

    }
    catch (const exception& e) {
        cout << "\n[ERROR] Restore failed - " << e.what() << "\n";
        return false;
    }
}

// List all available backups
vector<string> BackupManager::listBackups() {
    return getDirectoryList(backupDirectory);
}

// Delete a backup
bool BackupManager::deleteBackup(const string& backupName) {
    string backupPath = backupDirectory + backupName;

    if (!directoryExists(backupPath)) {
        cout << "\n[ERROR] Backup not found - " << backupName << "\n";
        return false;
    }

    cout << "\n";
    cout << "============================================================\n";
    cout << "               DELETE BACKUP                                \n";
    cout << "============================================================\n";
    cout << "\nBackup: " << backupName << "\n";
    cout << "\nAre you sure you want to DELETE this backup? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore();

    if (confirm != 'y' && confirm != 'Y') {
        cout << "\n[CANCELLED] Delete operation cancelled.\n";
        return false;
    }

    try {
        deleteDirectory(backupPath);
        cout << "\n[SUCCESS] Backup deleted successfully!\n";
        return true;
    }
    catch (const exception& e) {
        cout << "\n[ERROR] Delete failed - " << e.what() << "\n";
        return false;
    }
}

// Display backup menu
void BackupManager::displayBackupMenu() {
    cout << "\n" << string(60, '=') << "\n";
    cout << "         BACKUP & RECOVERY MENU\n";
    cout << string(60, '=') << "\n";
    cout << "1. Create New Backup\n";
    cout << "2. List All Backups\n";
    cout << "3. Restore Backup\n";
    cout << "4. Delete Backup\n";
    cout << "5. Return to Main Menu\n";
    cout << string(60, '-') << "\n";
}
void BackupManager::displayBackupsList(const vector<string>& backups) {
    cout << "\n" << string(90, '=') << "\n";
    cout << "                         AVAILABLE BACKUPS\n";
    cout << string(90, '=') << "\n";

    if (backups.empty()) {
        cout << "No backups found.\n";
        cout << string(90, '=') << "\n";
        return;
    }

    // Table header
    cout << left
        << setw(8) << "No."
        << setw(35) << "Backup Name"
        << setw(20) << "Date"
        << setw(20) << "Time" << "\n";
    cout << string(90, '-') << "\n";

    for (size_t i = 0; i < backups.size(); i++) {
        string backupName = backups[i];

        // Clean backup name - remove any trailing slashes or whitespace
        while (!backupName.empty() &&
            (backupName.back() == '\\' || backupName.back() == '/' ||
                backupName.back() == ' ' || backupName.back() == '\t')) {
            backupName.pop_back();
        }

        string formattedDate = "N/A";
        string formattedTime = "N/A";

        // Extract date and time from backup name
        // Expected format: backup_YYYYMMDD_HHMMSS (22 characters total)
        if (backupName.length() >= 22 && backupName.substr(0, 7) == "backup_") {
            // Extract the timestamp parts
            string dateStr = backupName.substr(7, 8);   // YYYYMMDD
            string timeStr = backupName.substr(16, 6);  // HHMMSS

            // Validate that both parts are numeric
            bool dateValid = (dateStr.length() == 8);
            bool timeValid = (timeStr.length() == 6);

            for (char c : dateStr) {
                if (c < '0' || c > '9') {
                    dateValid = false;
                    break;
                }
            }

            for (char c : timeStr) {
                if (c < '0' || c > '9') {
                    timeValid = false;
                    break;
                }
            }

            if (dateValid && timeValid) {
                // Format date as YYYY/MM/DD
                formattedDate = dateStr.substr(0, 4) + "/" +
                    dateStr.substr(4, 2) + "/" +
                    dateStr.substr(6, 2);

                // Format time as HH:MM:SS
                formattedTime = timeStr.substr(0, 2) + ":" +
                    timeStr.substr(2, 2) + ":" +
                    timeStr.substr(4, 2);
            }
        }

        // Display the row
        cout << left
            << setw(8) << (i + 1)
            << setw(35) << backupName
            << setw(20) << formattedDate
            << setw(20) << formattedTime << "\n";
    }

    cout << string(90, '=') << "\n";
}