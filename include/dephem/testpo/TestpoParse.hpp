#ifndef DEPHEM_TESTPO_TESTPO_PARSE_HPP
#define DEPHEM_TESTPO_TESTPO_PARSE_HPP

#include <vector>
#include <string>
#include <fstream>

#include "TestpoRecord.hpp"
#include "TestpoFilter.hpp"

namespace dph { namespace testpo { namespace details {

// Перемещение каретки потока к началу таблицы с testpo-записей.
bool GoToEOT(std::ifstream& testpoFile);

} } } // namespace dph::testpo::details


namespace dph { namespace testpo {

// Вектор записей testpo-файла.
typedef std::vector<Record> Records;

// Считать массив записей из testpo-файла.
Records ParseTestpoRecords(const std::string& testpoFilePath, 
    const Filter& filter = Filter());

} } // namespace dph::testpo


dph::testpo::Records 
dph::testpo::ParseTestpoRecords(const std::string& testpoFilePath, 
    const dph::testpo::Filter& filter)
{
	std::ifstream testpoFile(testpoFilePath.c_str());

    if(testpoFile.is_open())
    {
        bool found = details::GoToEOT(testpoFile);

        if(found)
        {
            Records records;

            for(std::string line; std::getline(testpoFile, line); )
            {
                const Record record(line);

                if(filter.pass(record))
                {
                    records.push_back(record);
                }
            }

            return records;
        }
    }

    return Records();
}

bool dph::testpo::details::GoToEOT(std::ifstream& testpoFile)
{
    if(testpoFile.is_open())
    {
        testpoFile.clear();
		testpoFile.seekg(std::ios::beg);
		
		std::string line;
		while (std::getline(testpoFile, line))
		{
			if (line.find("EOT") != std::string::npos)
			{
				return true;
			}
		}
    }

    return false;
}


#endif // DEPHEM_TESTPO_TESTPO_PARSE_HPP