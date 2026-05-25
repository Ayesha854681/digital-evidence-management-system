#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")

using namespace std;

class HashCalculator {
public:
    // Calculate SHA-256 hash of a file
    static string calculateFileHash(const string& filepath);

    // Verify file integrity by comparing hashes
    static bool verifyFileIntegrity(const string& filepath, const string& originalHash);

    // Check if file exists
    static bool fileExists(const string& filepath);

    // Get file size
    static long long getFileSize(const string& filepath);

private:
    // Convert binary hash to hex string
    static string toHexString(const unsigned char* data, size_t length);
};