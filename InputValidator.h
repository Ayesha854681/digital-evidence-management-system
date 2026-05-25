#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

#include <string>
#include <regex>
#include <algorithm>
#include <cctype>
#include <vector>
#include <iostream>
#include <limits>

using namespace std;

class InputValidator {
public:
    // ==================== ID VALIDATION ====================

    // Validate evidence ID format (E + year + 4 digits: E20240001)
    static bool isValidEvidenceID(const string& id) {
        if (id.empty()) return false;
        regex pattern("^E\\d{8}$");
        return regex_match(id, pattern);
    }

    // Validate case ID format (C + year + 4 digits: C20240001)
    static bool isValidCaseID(const string& id) {
        if (id.empty()) return false;
        regex pattern("^C\\d{8}$");
        return regex_match(id, pattern);
    }

    // ==================== HASH VALIDATION ====================

    // Validate SHA-256 hash format (64 hexadecimal characters)
    static bool isValidHash(const string& hash) {
        if (hash.length() != 64) return false;
        regex pattern("^[a-fA-F0-9]{64}$");
        return regex_match(hash, pattern);
    }

    // Validate hash with detailed error message
    static pair<bool, string> validateHashWithMessage(const string& hash) {
        if (hash.empty()) {
            return { false, "Hash cannot be empty" };
        }
        if (hash.length() != 64) {
            return { false, "SHA-256 hash must be exactly 64 characters (got " +
                    to_string(hash.length()) + ")" };
        }
        if (!regex_match(hash, regex("^[a-fA-F0-9]{64}$"))) {
            return { false, "Hash must contain only hexadecimal characters (0-9, a-f, A-F)" };
        }
        return { true, "Valid hash" };
    }

    // ==================== FILE SIZE VALIDATION ====================

    // Validate file size (must be positive and under 1TB)
    static bool isValidFileSize(long long size) {
        const long long MAX_SIZE = 1099511627776LL; // 1 TB
        return size > 0 && size <= MAX_SIZE;
    }

    // Validate file size with detailed message
    static pair<bool, string> validateFileSizeWithMessage(long long size) {
        if (size <= 0) {
            return { false, "File size must be positive" };
        }
        const long long MAX_SIZE = 1099511627776LL; // 1 TB
        if (size > MAX_SIZE) {
            return { false, "File size exceeds maximum limit of 1TB" };
        }
        return { true, "Valid file size" };
    }

    // ==================== STRING VALIDATION ====================

    // Validate description (must not be empty, max 500 chars)
    static bool isValidDescription(const string& desc) {
        if (desc.empty()) return false;
        if (desc.length() > 500) return false;
        // Check for printable characters
        for (char c : desc) {
            if (!isprint(static_cast<unsigned char>(c)) && c != '\n' && c != '\t') return false;
        }
        return true;
    }

    // Validate name fields (collector, person names)
    static bool isValidName(const string& name) {
        if (name.empty()) return false;
        if (name.length() > 100) return false;
        // Allow letters, spaces, hyphens, apostrophes
        regex pattern("^[a-zA-Z\\s\\-']+$");
        return regex_match(name, pattern);
    }

    // Validate general text fields (location, notes, etc.)
    static bool isValidTextField(const string& text, size_t maxLength = 500) {
        if (text.empty()) return false;
        if (text.length() > maxLength) return false;
        return true;
    }

    // ==================== PRIORITY VALIDATION ====================

    // Validate priority value (1-4)
    static bool isValidPriority(int priority) {
        return priority >= 1 && priority <= 4;
    }

    // ==================== FILE PATH VALIDATION ====================

    // Validate file path format
    static bool isValidFilePath(const string& path) {
        if (path.empty()) return false;
        if (path.length() > 260) return false; // Windows MAX_PATH

        // Check for invalid characters
        string invalidChars = "<>|\"*?";
        for (char c : invalidChars) {
            if (path.find(c) != string::npos) return false;
        }

        return true;
    }

    // ==================== SANITIZATION ====================

    // Sanitize description (remove dangerous characters)
    static string sanitizeDescription(const string& desc) {
        string sanitized = desc;

        // Remove pipe characters (used in CSV/data separation)
        sanitized.erase(remove(sanitized.begin(), sanitized.end(), '|'),
            sanitized.end());

        // Remove null terminators
        sanitized.erase(remove(sanitized.begin(), sanitized.end(), '\0'),
            sanitized.end());

        // Replace multiple newlines with single newline
        regex multipleNewlines("\n+");
        sanitized = regex_replace(sanitized, multipleNewlines, "\n");

        // Trim leading/trailing whitespace
        sanitized = trim(sanitized);

        return sanitized;
    }

    // Sanitize general text input
    static string sanitizeText(const string& text) {
        string sanitized = text;

        // Remove control characters except newline and tab
        sanitized.erase(remove_if(sanitized.begin(), sanitized.end(),
            [](unsigned char c) { return iscntrl(c) && c != '\n' && c != '\t'; }),
            sanitized.end());

        // Remove pipe characters
        sanitized.erase(remove(sanitized.begin(), sanitized.end(), '|'),
            sanitized.end());

        // Trim whitespace
        sanitized = trim(sanitized);

        return sanitized;
    }

    // Sanitize file path
    static string sanitizeFilePath(const string& path) {
        string sanitized = path;

        // Remove quotes
        sanitized.erase(remove(sanitized.begin(), sanitized.end(), '"'),
            sanitized.end());
        sanitized.erase(remove(sanitized.begin(), sanitized.end(), '\''),
            sanitized.end());

        // Trim whitespace
        sanitized = trim(sanitized);

        return sanitized;
    }

    // ==================== EMAIL VALIDATION ====================

    // Validate email format (for collector email, etc.)
    static bool isValidEmail(const string& email) {
        if (email.empty()) return false;
        regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        return regex_match(email, pattern);
    }

    // ==================== DATE VALIDATION ====================

    // Validate date string format (YYYY-MM-DD)
    static bool isValidDateFormat(const string& date) {
        if (date.empty()) return false;
        regex pattern("^\\d{4}-\\d{2}-\\d{2}$");
        if (!regex_match(date, pattern)) return false;

        // Parse and validate actual date values
        int year, month, day;
        if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
            return false;
        }

        // Basic date validation
        if (year < 2000 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;

        return true;
    }

    // ==================== NUMERIC INPUT VALIDATION ====================

    // Safe integer input with range validation
    static bool getValidatedInt(int& value, int min, int max) {
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value >= min && value <= max;
    }

    // Safe long long input with range validation
    static bool getValidatedLongLong(long long& value, long long min, long long max) {
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value >= min && value <= max;
    }

    // ==================== ERROR DISPLAY ====================

    // Display validation error with suggestion
    static void displayValidationError(const string& fieldName,
        const string& errorMessage,
        const string& suggestion = "") {
        cout << "\n[VALIDATION ERROR] " << fieldName << ": " << errorMessage << "\n";
        if (!suggestion.empty()) {
            cout << "[SUGGESTION] " << suggestion << "\n";
        }
    }

    // Comprehensive input validation report
    static void displayValidationReport(const vector<pair<string, string>>& errors) {
        if (errors.empty()) {
            cout << "\n[✓] All validation checks passed!\n";
            return;
        }

        cout << "\n========================================\n";
        cout << "  VALIDATION ERRORS (" << errors.size() << " found)\n";
        cout << "========================================\n";
        for (size_t i = 0; i < errors.size(); i++) {
            cout << (i + 1) << ". " << errors[i].first << ":\n";
            cout << "   " << errors[i].second << "\n";
        }
        cout << "========================================\n";
    }

private:
    // ==================== HELPER FUNCTIONS ====================

    // Trim whitespace from string
    static string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }
};

#endif // INPUTVALIDATOR_H