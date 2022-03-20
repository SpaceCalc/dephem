#ifndef DEPHEM_H
#define DEPHEM_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <fstream>
#include <cstring>
#include <stdint.h>
#include <string>
#include <map>
#include <vector>

namespace dph {

// Индексы тел.
enum Body
{
    B_MERCURY = 1,
    B_VENUS   = 2,
    B_EARTH   = 3,
    B_MARS    = 4,
    B_JUPITER = 5,
    B_SATURN  = 6,
    B_URANUS  = 7,
    B_NEPTUNE = 8,
    B_PLUTO   = 9,
    B_MOON    = 10,
    B_SUN     = 11,
    B_SSBARY  = 12, // Барицентр Солнечной Системы.
    B_EMBARY  = 13  // Барицентр системы Земля-Луна.
};

// Индексы элементов.
enum Item {
    I_MERCURY = 0,
    I_VENUS   = 1,
    I_EMBARY  = 2,
    I_MARS    = 3,
    I_JUPITER = 4,
    I_SATURN  = 5,
    I_URANUS  = 6,
    I_NEPTUNE = 7,
    I_PLUTO   = 8,
    I_MOON    = 9,
    I_SUN     = 10,
    I_EN      = 11,
    I_LML     = 12,
    I_LMAV    = 13,
    I_TTMTDB  = 14
};

// Класс для работы с выпусками DE-эфемерид JPL в бинарном формате.
class DevelopmentEphemeris
{
public:
    DevelopmentEphemeris();

    // Конструктор по пути к бинарному файлу эфемерид.
    DevelopmentEphemeris(const std::string& filePath);

    // Конструктор копирования.
    DevelopmentEphemeris(const DevelopmentEphemeris& other);

    // Оператор копирования.
    DevelopmentEphemeris& operator=(const DevelopmentEphemeris& other);

    // Открыть файл эфемерид.
    bool open(const std::string& filePath);

    // Положение target относительно center на момент времени jed.
    bool bodyPosition(int target, int center, double jed, double pos[3]) const;

    // Положение и скорость target относительно center на момент времени jed.
    bool bodyState(int target, int center, double jed, double state[6]) const;

    // Значение отдельного элемента item на момент времени jed.
    bool item(int item, double jed, double* result) const;

    // Готовность объекта к использованию.
    bool isOpen() const;

    // Первая доступная дата для рассчётов.
    double beginJed() const;

    // Последняя доступная дата для рассчётов.
    double endJed() const;

    // Номер выпуска.
    int index() const;

    // Строковая информация о выпуске.
    std::string label() const;

    // Значение константы по её имени.
    double constant(const std::string& name, bool* ok = nullptr) const;

private:
    // Формат эфемерид.
    static const size_t LABELS_COUNT = 3;      // Строк в подписи.
    static const size_t LABEL_SIZE = 84;       // Символов в строке подписи.
    static const size_t CNAME_SIZE = 6;        // Символов в имени константы.
    static const size_t CCOUNT_MAX_OLD = 400;  // Кол-во констант (ст. формат).
    static const size_t CCOUNT_MAX_NEW = 1000; // Кол-во констант (нов. формат).
    static const size_t MAX_POLY_SIZE = 15;    // Макс. размер полиномов.

    // Работа с файлом
    std::string m_filePath;        // Путь к файлу эфемерид.
    mutable std::ifstream m_file;  // Поток чтения файла.
    mutable std::vector<double> m_buffer; // Буффер блока с коэффициентами.

    // Параметры эфемерид.
    std::string m_label; // Строковая информация о выпуске.
    int m_index;         // Номерная часть индекса выпуска.
    double m_beginJed;   // Дата начала выпуска (JED).
    double m_endJed;     // Дата окончания выпуска (JED).
    double m_blockSpan;  // Временная протяжённость блока.
    int m_keys[15][3];   // Ключи поиска коэффициентов.
    double m_au;         // Астрономическая единица (км).
    double m_emrat;      // Отношение массы Земли к массе Луны.
    int m_ncoeff;        // Количество коэффициентов в блоке.

    std::map<std::string, double> m_constants; // Константы выпуска.


    // Обрезать повторяющиеся пробелы (' ') с конца массива символов "s"
    // размера "arraySize".
    static std::string cutBackSpaces(const char* s, size_t size);

    // Приведение объекта к изначальному состоянию.
    void clear();

    // Копирование информации из объекта "other" в текущий объект.
    void copyHere(const DevelopmentEphemeris& other);

    // Чтение файла.
    bool read();

    // Заполнение буффера "m_buffer" коэффициентами требуемого блока.
    bool fillBuffer(size_t blockNum) const;

    bool body(int resType, double jed, int target, int center,
        double* res) const;

    // Базовый элемент.
    bool baseItem(int resType, double jed, int baseItem, double* res) const;

    // Земля относительно барицентра Солнечной Системы.
    bool ssbaryEarth(int resType, double jed, double* res) const;

    // Луна относительно барицентра Солнечной Системы.
    bool ssbaryMoon(int resType, double jed, double* res) const;

}; // class EphemerisRelease

} // namespace dph

#endif // DEPHEM_H
