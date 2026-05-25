#include "HashCalculator.h"
#include <iostream>

// Calculate SHA-256 hash using Windows CryptoAPI
string HashCalculator::calculateFileHash(const string& filepath) {
    // Check if file exists
    ifstream file(filepath, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open file: " << filepath << endl;
        return "";
    }

    // Initialize crypto provider
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        cerr << "Error: CryptAcquireContext failed!" << endl;
        return "";
    }

    // Create hash object
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        cerr << "Error: CryptCreateHash failed!" << endl;
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Read file in chunks and update hash
    const size_t BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];

    while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
        if (!CryptHashData(hHash, (BYTE*)buffer, (DWORD)file.gcount(), 0)) {
            cerr << "Error: CryptHashData failed!" << endl;
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            return "";
        }
    }

    file.close();

    // Get hash value
    BYTE hashValue[32]; // SHA-256 produces 32 bytes
    DWORD hashLen = 32;

    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashValue, &hashLen, 0)) {
        cerr << "Error: CryptGetHashParam failed!" << endl;
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    // Convert to hex string
    string result = toHexString(hashValue, hashLen);

    // Cleanup
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return result;
}

// Verify file integrity
bool HashCalculator::verifyFileIntegrity(const string& filepath, const string& originalHash) {
    if (originalHash.empty()) {
        cerr << "Error: Original hash is empty!" << endl;
        return false;
    }

    string currentHash = calculateFileHash(filepath);

    if (currentHash.empty()) {
        return false;
    }

    // Compare hashes (case-insensitive)
    if (currentHash.length() != originalHash.length()) {
        return false;
    }

    for (size_t i = 0; i < currentHash.length(); i++) {
        if (tolower(currentHash[i]) != tolower(originalHash[i])) {
            return false;
        }
    }

    return true;
}

// Check if file exists
bool HashCalculator::fileExists(const string& filepath) {
    ifstream file(filepath);
    return file.good();
}

// Get file size
long long HashCalculator::getFileSize(const string& filepath) {
    ifstream file(filepath, ios::binary | ios::ate);
    if (!file) return -1;
    return file.tellg();
}

// Convert binary to hex string
string HashCalculator::toHexString(const unsigned char* data, size_t length) {
    stringstream ss;
    ss << hex << setfill('0');

    for (size_t i = 0; i < length; i++) {
        ss << setw(2) << static_cast<int>(data[i]);
    }

    return ss.str();
}