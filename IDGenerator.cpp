#define _CRT_SECURE_NO_WARNINGS
#include "IDGenerator.h"
#include <ctime>
#include <fstream>
#include <cstdio>

using namespace std;

// Static member initialization
int IDGenerator::evidenceCounter = 0;
int IDGenerator::caseCounter = 0;

string IDGenerator::generateEvidenceID() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    char buffer[20];
    sprintf(buffer, "E%04d%04d", t->tm_year + 1900, ++evidenceCounter);
    saveCounters();
    return string(buffer);
}

string IDGenerator::generateCaseID() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    char buffer[20];
    sprintf(buffer, "C%04d%04d", t->tm_year + 1900, ++caseCounter);
    saveCounters();
    return string(buffer);
}

void IDGenerator::loadCounters() {
    ifstream in("id_counters.txt");
    if (in.is_open()) {
        in >> evidenceCounter >> caseCounter;
        in.close();
    }
}

void IDGenerator::saveCounters() {
    ofstream out("id_counters.txt");
    out << evidenceCounter << " " << caseCounter;
    out.close();
}
