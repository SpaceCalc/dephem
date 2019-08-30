#ifndef DEPHEM_TESTPO_TESTPO_COMPARE_HPP
#define DEPHEM_TESTPO_TESTPO_COMPARE_HPP

#include <cmath> // fabs
#include <string> 

#include "TestpoRecord.hpp"
#include "TestpoParse.hpp"
#include "../help.hpp"
#include "../../../dephem/include/dephem.hpp"

namespace dph { namespace testpo {

namespace details
{
    double TestpoUnitsCorrection(unsigned itemIndex, 
        unsigned calculationsResult, unsigned componentsCount, double AU)
    {
        using namespace dph::help;
        
        double correction = 1;

        if(itemIndex <= Body::EMBARY)
        {
            correction = AU;
        }
        if(calculationsResult == Calculate::STATE)
        {
            correction /= 86400;
        }

        return correction;
    }
}

extern const size_t USE_TESTPO_UNITS = 1 << 15;

double CalculateRecord(const Record& record, 
    const EphemerisRelease& ephemerisRelease, size_t unitsMode = 0)
{
    using namespace dph::help;
   
    unsigned componentsCount = dph::help::ComponentsCount(record.target());

    unsigned calculationsResult = record.component() > componentsCount ? 
                                    Calculate::STATE : Calculate::POSITION;

    double resultArray[6];

    if(record.target() <= Body::EMBARY)
    {
        ephemerisRelease.calculateBody(calculationsResult, record.target(), 
            record.center(), record.date(), resultArray);
    }
    else if(record.target() <= Other::TTmTDB)
    {
        ephemerisRelease.calculateOther(calculationsResult, record.target(), 
            record.date(), resultArray);
    }

    double correction = 1;
    if(unitsMode == USE_TESTPO_UNITS)
    {
        correction = details::TestpoUnitsCorrection(record.target(), 
            calculationsResult, componentsCount, 
                ephemerisRelease.constant("AU"));
    }

    return resultArray[record.component() - 1] / correction;
}

bool SimpleTestpoCompare(const EphemerisRelease& ephem, 
    const Records& records, double allowableError)
{
    Filter filter;
    filter.fit(ephem);
    
    Records::const_iterator record;
    for(record = records.begin(); record != records.end(); ++record)
    {
        if(filter.pass(*record))
        {
            double value = CalculateRecord(*record, ephem, USE_TESTPO_UNITS);

            double error = std::fabs(value - record->value());

            if(error > allowableError)
            {
                return false;
            }
        }
    }

    return true;
}

bool SimpleTestpoCompare(const EphemerisRelease& ephem, 
    const std::string& testpoFilePath, double allowableError)
{
    return SimpleTestpoCompare(ephem, ParseTestpoRecords(testpoFilePath), 
        allowableError);
}

bool SimpleTestpoCompare(const std::string& ephemerisFilePath, 
    const std::string& testpoFilePath, double allowableError)
{
    return SimpleTestpoCompare(EphemerisRelease(ephemerisFilePath), 
        testpoFilePath, allowableError);
}

} } // namespace dph::testpo

#endif // DEPHEM_TESTPO_TESTPO_COMPARE_HPP