#ifndef DEPHEM_LNX_FORMAT_PARSE_HPP
#define DEPHEM_LNX_FORMAT_PARSE_HPP

#include <stdint.h> // uint32_t 
#include <fstream>

// =============================== ПРОТОТИПЫ ================================ //

namespace dph { namespace lnx {

struct LnxHeader;

LnxHeader ParseLnxHeader(const std::string& lnxFilePath);
LnxHeader ParseLnxHeader(std::ifstream& lnxFile);

void WriteLnxHeader(const LnxHeader& header, const std::string& lnxFilePath);
void WriteLnxHeader(const LnxHeader& header, std::fstream& lnxFile);


namespace details
{

size_t BaseComponentsCount(unsigned baseItemIndex);

size_t CoeffsPerBlock(const uint32_t keys[15][3]);

std::ios::openmode un_mode(const std::string& filePath);

} // namespace dph::lnx::details

} } // namespace dph::lnx

// ============================== РЕАЛИЗАЦИЯ  =============================== //

struct dph::lnx::LnxHeader
{
    char        label[3][84];
    uint32_t    index;
    double      startDate;
    double      endDate;
    double      span;
    double      au;
    double      emrat;
    uint32_t    keys[15][3];
    uint32_t    constantsCount;
    char        constantsNames[1000][6];
    double      constantsValues[1000];
};

dph::lnx::LnxHeader dph::lnx::ParseLnxHeader(const std::string& lnxFilePath)
{
    std::ifstream lnxFile(lnxFilePath.c_str(), std::ios::binary);

    return ParseLnxHeader(lnxFile);
}

dph::lnx::LnxHeader dph::lnx::ParseLnxHeader(std::ifstream& lnxFile)
{
    if(lnxFile.is_open() == false)
    {
        return LnxHeader();
    }
    
    lnxFile.seekg(std::ios::beg);
    
    LnxHeader header = {};

    lnxFile.read((char*)&header.label, 84 * 3);        
    lnxFile.read((char*)&header.constantsNames, 400 * 6);
    lnxFile.read((char*)&header.startDate, 8);
    lnxFile.read((char*)&header.endDate, 8);
    lnxFile.read((char*)&header.span, 8);
    lnxFile.read((char*)&header.constantsCount, 4);
    lnxFile.read((char*)&header.au, 8);
    lnxFile.read((char*)&header.emrat, 8);
    lnxFile.read((char*)&header.keys, (12 * 3) * 4);
    lnxFile.read((char*)&header.index,4);
    lnxFile.read((char*)&header.keys[12], 3 * 4);

    if(header.constantsCount > 400)
    {
        lnxFile.read(header.constantsNames[400], 
            (header.constantsCount - 400) * 6);
    }

    lnxFile.read((char*)&header.keys[13], (3 * 2) * 4);

    std::streampos constantsValuesPos = 
        static_cast<std::streampos>(details::CoeffsPerBlock(header.keys) * 8);
        
    lnxFile.seekg(constantsValuesPos, std::ios::beg);

    lnxFile.read((char*)&header.constantsValues, header.constantsCount * 8);        

    return header;
}

void dph::lnx::WriteLnxHeader(const LnxHeader& header, const std::string& lnxFilePath)
{
    std::fstream lnxFile(lnxFilePath.c_str(), details::un_mode(lnxFilePath));

    WriteLnxHeader(header, lnxFile);    
}

void dph::lnx::WriteLnxHeader(const LnxHeader& header, std::fstream& lnxFile)
{
    if(lnxFile.is_open() == false)
    {
        return;
    }

    lnxFile.seekp(std::ios::beg);

    lnxFile.write((char*)&header.label, 84 * 3);        
    lnxFile.write((char*)&header.constantsNames, 400 * 6);
    lnxFile.write((char*)&header.startDate, 8);
    lnxFile.write((char*)&header.endDate, 8);
    lnxFile.write((char*)&header.span, 8);
    lnxFile.write((char*)&header.constantsCount, 4);
    lnxFile.write((char*)&header.au, 8);
    lnxFile.write((char*)&header.emrat, 8);
    lnxFile.write((char*)&header.keys, (12 * 3) * 4);
    lnxFile.write((char*)&header.index,4);
    lnxFile.write((char*)&header.keys[12], 3 * 4);

    if(header.constantsCount > 400)
    {
        lnxFile.write(header.constantsNames[400], 
            (header.constantsCount - 400) * 6);
    }

    lnxFile.write((char*)&header.keys[13], (3 * 2) * 4);

    std::streampos constantsValuesPos = 
        static_cast<std::streampos>(details::CoeffsPerBlock(header.keys) * 8);
        
    lnxFile.seekp(constantsValuesPos, std::ios::beg);

    lnxFile.write((char*)&header.constantsValues, header.constantsCount * 8);
}

size_t dph::lnx::details::BaseComponentsCount(unsigned baseItemIndex)
{
    return baseItemIndex > 14 ? 0 :
            baseItemIndex == 14 ? 1 :
                baseItemIndex == 11 ? 2 : 3;
}

size_t dph::lnx::details::CoeffsPerBlock(const uint32_t keys[15][3])
{
    size_t count = 2;

    for(int i = 0; i < 15; ++i)
    {
        count += BaseComponentsCount(i) * keys[i][1] * keys[i][2];
    }

    return count;
}

std::ios::openmode dph::lnx::details::un_mode(const std::string& filePath)
{
    using namespace std;

    ios::openmode write_if_exist = ios::binary | ios::out | ios::in;
    ios::openmode write_if_create = ios::binary | ios::out;
    
    return fstream(filePath.c_str(), write_if_exist).is_open() ?
        write_if_exist : write_if_create;
}

#endif // DEPHEM_LNX_FORMAT_PARSE_HPP