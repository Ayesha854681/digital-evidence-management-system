#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <string>

class IDGenerator {
private:
    static int evidenceCounter;
    static int caseCounter;

public:
    static std::string generateEvidenceID();
    static std::string generateCaseID();

    static void loadCounters();
    static void saveCounters();
};

#endif

