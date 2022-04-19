#include <iostream>
#include <sstream>
#include <cmath>
#include <dephem/dephem.h>

int main(int argc, char *argv[])
{
    using namespace dph;

    std::cout << "[DEPHEM TESTPO FILE TEST]" << std::endl;

    if (argc < 3)
    {
        std::cerr << "invalid arguments count\n";
        return 1;
    }

    std::string ephemPath = argv[1];
    std::string testpoPath = argv[2];

    std::cout << "ephemris file: " << ephemPath << std::endl;
    std::cout << "testpo file:   " << testpoPath << std::endl;
    if (argc >= 4)
        std::cout << "report file:   " << argv[3] << std::endl;

    DevelopmentEphemeris ephem;

    if (!ephem.open(ephemPath))
    {
        std::cerr << "unable to open " << ephemPath << " as ephemeris file\n";
        return 1;
    }

    auto report = ephem.testpoReport(testpoPath);

    if (!report.ok())
    {
        std::cerr << report.errmsg() << std::endl;
        return 1;
    }

    size_t skippedCount = 0;
    size_t passedCount = 0;
    size_t invalidCount = 0;

    std::vector<TestpoReport::TestCase> invalid;

    for (const auto& _case : report.testCases())
    {
        switch (_case.caseCode) {
        case 0: passedCount++; break;
        case 1: skippedCount++; break;
        default: invalidCount++; invalid.push_back(_case); break;
        }
    }

    auto warnings = report.warnings(1E-13);
    size_t warningsCount = warnings.size();

    passedCount -= warningsCount;

    std::cout << "[TESTING DONE]" << std::endl;
    std::cout << "test cases: " << report.testCases().size() << std::endl;
    std::cout << "skipped:    " << skippedCount << std::endl;
    std::cout << "passed:     " << passedCount << std::endl;
    std::cout << "invalid:    " << invalidCount << std::endl;
    std::cout << "warnings:   " << warningsCount << std::endl;

    bool hasError = false;

    if (argc >= 4)
    {
        auto reportPath = argv[3];

        int writeErr = report.write(reportPath);

        if (writeErr)
        {
            std::cerr << "unable to create " << reportPath << std::endl;
            hasError = true;
        }
    }

    if (!invalid.empty())
    {
        std::cerr << "[INVALID]" << std::endl;

        for (const auto& _case : invalid)
            std::cerr << _case << std::endl;

        hasError = true;
    }

    if (!warnings.empty())
    {
        std::cerr << "[WARNINGS]" << std::endl;

        for (const auto& _case : warnings)
            std::cerr << _case << std::endl;

        hasError = true;
    }

    return hasError ? 1 : 0;
}
