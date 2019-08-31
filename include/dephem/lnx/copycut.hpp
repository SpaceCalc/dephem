#ifndef DEPHEM_LNX_FORMAT_GENERATE
#define DEPHEM_LNX_FORMAT_GENERATE

#include "parse.hpp"

// =============================== ПРОТОТИПЫ ================================ //

namespace dph { namespace lnx {

// Генерация файла эфемерид lnx-формата на базе другого (по путям к файлам).
void CopyCutLnxRelease(const std::string& baseLnxFilePath, 
    const std::string& newLnxFilePath, double startDate, double endDate);

// Генерация файла эфемерид lnx-формата на базе другого (по потокам).
void CopyCutLnxRelease(std::ifstream& baseLnxFile, std::fstream& newLnxFile,
    double startDate, double endDate);

} } // namespace dph::lnx

// ============================== РЕАЛИЗАЦИЯ  =============================== //

void dph::lnx::CopyCutLnxRelease(const std::string& baseLnxFilePath, 
    const std::string& newLnxFilePath, double startDate, double endDate)
{
    std::ifstream baseLnxFile(baseLnxFilePath.c_str(), std::ios::binary);

    std::fstream newLnxFile(newLnxFilePath.c_str(), 
        details::un_mode(newLnxFilePath));

    CopyCutLnxRelease(baseLnxFile, newLnxFile, startDate, endDate);
}

void dph::lnx::CopyCutLnxRelease(std::ifstream& baseLnxFile, 
std::fstream& newLnxFile, double startDate, double endDate)
{
    // Базовые проверки:
    if(baseLnxFile.is_open() == false)
    {
        return;
    }
    else if(newLnxFile.is_open() == false)
    {
        return;
    }
    else if(startDate >= endDate)
    {
        return;
    }

    // Чтение заголовочной информации:
    LnxHeader header(ParseLnxHeader(baseLnxFile));

    // Проверка дат:
    if(startDate < header.startDate || endDate > header.endDate)
    {
        return;
    }

    // Порядковые номера граничных блоков:
    size_t firstBlockNum = static_cast<size_t>((startDate - header.startDate) / 
        header.span);
    size_t lastBlockNum = static_cast<size_t>((endDate - header.startDate) / 
        header.span);

    // Обновление заголовочной информации:
    header.endDate = header.startDate + lastBlockNum * header.span;
    header.startDate = header.startDate + firstBlockNum * header.span;

    // Запись обновлённой заголовочной информации в новый файл:
    WriteLnxHeader(header, newLnxFile);

    // Размер одного блока в байтах:
    std::streampos blockSizeBytes = static_cast<std::streampos>(
        details::CoeffsPerBlock(header.keys) * 8);

    // Синхронизация позиций копирования в потоках:
    baseLnxFile.seekg(blockSizeBytes * (firstBlockNum + 2), std::ios::beg);
    newLnxFile.seekp(blockSizeBytes * 2, std::ios::beg);

    // Промежуточный буфер:
    char* buffer = new char[blockSizeBytes];
    
    // Копирование блоков:
    for(size_t i = firstBlockNum; i <= lastBlockNum; ++i)
    {
        baseLnxFile.read(buffer, blockSizeBytes);
        newLnxFile.write(buffer, blockSizeBytes);
    }
    
    delete[] buffer;    
}

#endif // DEPHEM_LNX_FORMAT_GENERATE