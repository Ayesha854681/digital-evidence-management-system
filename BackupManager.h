#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <fstream>
#include <direct.h>  // For _mkdir on Windows
#include <io.h>      // For _access on Windows
#include <sys/stat.h>
#include <windows.h> // For file operations

using namespace std;

class BackupManager {
private:
    string backupDirectory;

    // Helper functions
    bool createDirectory(const string& path);
    bool copyFile(const string& source, const string& destination);
    bool copyDirectory(const string& source, const string& destination);
    bool directoryExists(const string& path);
    bool fileExists(const string& path);
    vector<string> getDirectoryList(const string& path);
    void deleteDirectory(const string& path);

public:
    BackupManager();

    // Main backup operations
    bool createBackup();
    bool restoreBackup(const string& backupName);
    vector<string> listBackups();
    bool deleteBackup(const string& backupName);

    // Display functions
    void displayBackupMenu();
    void displayBackupsList(const vector<string>& backups);
};

#endif