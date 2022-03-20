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

namespace dph
{

// ************************************************************************** //
//                                   Body                                     //
//                                                                            //
//                                Индексы тел                                 //
// -------------------------------------------------------------------------- //
//                                 Описание                                   //
// -------------------------------------------------------------------------- //
// Вспомогательный класс, хранящий значения параметров для метода             //
// dph::EphemerisReelase::calculateBody(...).                                 //
//                                                                            //
// Значения хранимых констант соответствуют индексам тел, для которых         //
// можно получить результат вычислений.                                       //
//                                                                            //
// ************************************************************************** //
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

// ************************************************************************** //
//                                   Other                                    //
//                                                                            //
//                         Индексы прочих элементов                           //
// -------------------------------------------------------------------------- //
//                                 Описание                                   //
// -------------------------------------------------------------------------- //
// Вспомогательный класс, хранящий значения параметров для метода             //
// dph::EphemerisReelase::calculateOther(...).                                //
//                                                                            //
// Значения хранимых констант соответствуют индексам прочих элементов, для    //
// которых можно получить результат вычислений.                               //
//                                                                            //
// ************************************************************************** //
enum Other
{
    O_EARTH_NUTATIONS               = 14,
    O_LUNAR_MANTLE_LIBRATION        = 15,
    O_LUNAR_MANTLE_ANGULAR_VELOCITY = 16,
    O_TTmTDB                        = 17
};

// ************************************************************************** //
//                                 ResType                                  //
//                                                                            //
//                      Индексы результатов вычислений                        //
// -------------------------------------------------------------------------- //
//                                 Описание                                   //
// -------------------------------------------------------------------------- //
// Вспомогательный класс, хранящий значения параметров для методов:           //
//    dph::EphemerisReelase::calculateBody(...),                              //
//    dph::EphemerisReelase::calculateOther(...).                             //
//                                                                            //
// Значения хранимых констант соответствуют индексам результатов вычислений,  //
// которые можно получить.                                                    //
//                                                                            //
// ************************************************************************** //
enum ResType
{
    RES_POS   = 0,
    RES_STATE = 1
};

// ************************************************************************** //
//                             EphemerisRelease                               //
//                                                                            //
//      Класс для работы с выпусками DE-эфемерид JPL в бинарном формате       //
// -------------------------------------------------------------------------- //
//                                 Описание                                   //
// -------------------------------------------------------------------------- //
// Объект данного класса является представлением требуемого выпуска           //
// DE-эфемерид.                                                               //
//                                                                            //
// Возможности:                                                               //
//     - Вычисление значений элементов, хранящихся в выпуске эфемерид.        //
//     - Получение общей информации о выпуске эфемерид.                       //
//     - Хранение и доступ к константам, хранящихся в выпуске эфемерид.       //
//                                                                            //
// ************************************************************************** //
class EphemerisRelease
{
public:

// ------------------------ Стандартные методы класса ----------------------- //

    // Конструктор по пути к бинарному файлу эфемерид.
    // -----------------------------------------------
    // Чтение файла, проверка полученных значений.
    explicit EphemerisRelease(const std::string& filePath);

    // Конструктор копирования.
    // ------------------------
    // Проверка полученных значений и доступ к файлу.
    // При неудачной проверке объект очищается.
    EphemerisRelease(const EphemerisRelease& other);

    // Оператор копирования.
    // ---------------------
    // Проверка полученных значений и доступ к файлу.
    // При неудачной проверке объект очищается.
    EphemerisRelease& operator=(const EphemerisRelease& other);

// ---------------------------- Методы вычислений ----------------------------//

    // Получить значение радиус-вектора (или вектора состояния) выбранного
    // тела относительно другого на заданный момент времени.
    // -------------------------------------------------------------------
    // Параметры метода:
    //
    // - resType : Индекс результата вычислений.
    //             Используй dph::Calculate.
    //
    // - jed     : Момент времени на который требется произвести
    //             вычисления в формате Юлианской Эфемеридной Даты
    //             (Julian Epehemris Date).
    //             Принадлежит промежутку: [startDate : endDate].
    //
    // - targety : Порядковый номер искомого тела
    //             Используй dph::Body.
    //
    // - center  : Порядковый номер центрального тела.
    //             Используй dph::Body.
    //
    // - res     : Указатель на массив для результата вычислений.
    //             Убедись, что он имеент минимальный допустимый
    //             размер для выбранного результата вычислений.
    // -----------------
    // Примечание: если в метод поданы неверные параметры, то он просто
    // прервётся.
    // -----------------
    void calculateBody(ResType resType, double jed,
        Body target, Body center, double* res) const;

    // Получить значение(-я) прочих элементов, хранящихся в выпуске эфемерид,
    // на заданный момент времени.
    // -----------------
    // Параметры метода:
    //
    // - resType : Индекс результата вычислений
    //             Используй dph::Calculate.
    //
    // - jed     : Момент времени на который требется произвести
    //             вычисления в формате Юлианской Эфемеридной Даты
    //             (Julian Epehemris Date).
    //  Принадлежит промежутку: [startDate : endDate].
    //
    // - item    : Порядковый номер искомого элемента
    //             Используй dph::Other.
    //
    // - res     : Указатель на массив для результата вычислений.
    //             Убедись, что  он имеент минимальный допустимый
    //             размер для выбранного результата вычислений.
    // -----------------
    // Примечания:
    // 1. Если в метод поданы неверные параметры, то он просто прервётся.
    // 2. Не всегда в выпуске эфемерид хранится запрашиваемое тело, убедись
    //    в его наличии перед запросом.
    // -----------------
    void calculateOther(ResType resType, double jed, Other item,
        double* res) const;


// --------------------------------- ГЕТТЕРЫ -------------------------------- //

    // Готовность объекта к использованию.
    bool isReady() const;

    // Первая доступная дата для рассчётов.
    double beginJed() const;

    // Последняя доступная дата для рассчётов.
    double endJed() const;

    // Номер выпуска.
    uint32_t index() const;

    // Строковая информация о выпуске.
    std::string label() const;

    // Значение константы по её имени.
    double constant(const std::string& constantName) const;

private:

// -------------------------- Внутренние значения --------------------------- //

// .......................... Формат DE-эфемерид ............................ //

    // Кол-во строк Общей Информации (ОИ).
    static const size_t RLS_LABELS_COUNT = 3;

    // Кол-во символов в строке ОИ.
    static const size_t RLS_LABEL_SIZE = 84;

    // Кол-во символов в имени константы.
    static const size_t CNAME_SIZE = 6;

    // Кол-во констант (стар. формат).
    static const size_t CCOUNT_MAX_OLD = 400;

    // Кол-во констант (нов. формат).
    static const size_t CCOUNT_MAX_NEW = 1000;

// ............................ Состояние объекта ............................//

    bool m_ready; // Готовность объекта к работе.

// ........................... Работа с файлом ...............................//

    std::string m_filePath;        // Путь к файлу эфемерид.
    mutable std::ifstream m_file;  // Поток чтения файла.

// ..................... Значения, считанные из файла ....................... //

    std::string m_label;    // Строковая информация о выпуске.
    uint32_t m_index;       // Номерная часть индекса выпуска.
    double m_beginJed;      // Дата начала выпуска (JED).
    double m_endJed;        // Дата окончания выпуска (JED).
    double m_blockTimeSpan; // Временная протяжённость блока.
    uint32_t m_keys[15][3]; // Ключи поиска коэффициентов.
    double m_au;            // Астрономическая единица (км).
    double m_emrat;         // Отношение массы Земли к массе Луны.

    std::map<std::string, double> m_constants; // Константы выпуска.

// ......... Значения, дополнительно определённные внутри объекта ........... //

    size_t m_blocksCount;     // Количество блоков в файле.
    uint32_t m_ncoeff;        // Количество коэффициентов в блоке.
    double m_emrat2;          // Отношение массы Луны к массе Земля-Луна.
    double m_dimensionFit;    // Значение для соблюдения размерности.
    size_t m_blockSize_bytes; // Размер блока в байтах.

// .............. Динамические массивы для работы с выпуском ................ //

    mutable std::vector<double> m_buffer; // Буффер блока с коэффициентами.
    mutable std::vector<double> m_poly;   // Значения полиномов.
    mutable std::vector<double> m_dpoly;  // Значения производных полиномов.


// -------------------- Приватные методы работы объекта --------------------- //

// .......................... Статические методы ............................ //

    // Обрезать повторяющиеся пробелы (' ') с конца массива символов "s"
    // размера "arraySize".
    static std::string cutBackSpaces(const char* s, size_t size);

// .............. Дополнения к стандартным публичным методам ................ //

    // Приведение объекта к изначальному состоянию.
    void clear();

    // Копирование информации из объекта "other" в текущий объект.
    void copyHere(const EphemerisRelease& other);

// ........... Чтение файла и инициализация внутренних значений ............. //

    //  Чтение файла.
    void readAndPackData();

    // Дополнительные вычисления после чтения файла.
    void additionalCalculations();

    // Проверка значений, хранящихся в объекте и проверка файла.
    bool isDataCorrect() const;

    // Проверка начальных и конечных дат всех блоков в файле.
    // Подтверждает целостность файла и доступность всех коэффициентов.
    // Входит в состав проверки isDataCorrect().
    bool check_blocksDates() const;

    // Заполнение буффера "m_buffer" коэффициентами требуемого блока.
    void fillBuffer(size_t blockNum) const;

// .............................. Вычисления ................................ //

    // Интерполяция компонент выбранного базового элемента.
    void interpolatePosition(double normalizedTime, unsigned baseItem,
        const double* coeffs, unsigned componentsCount,
        double* res) const;

    // Интерполяция компонент и их производных выбранного базового элемента.
    void interpolateState(double normalizedTime, unsigned baseItem,
        const double* coeffs, unsigned componentsCount,
        double* res) const;

    // Получить значения требуемых компонент базового элемента на выбранный
    // момент времени.
    void calculateBaseItem(unsigned resType, double jed, unsigned baseItem,
        double* res) const;

    // Получить значение радиус-вектора (или вектора состояния) Земли
    // относительно барицентра Солнечной Системы.
    void calculateBaseEarth(unsigned resType, double jed, double* res) const;

    // Получить значение радиу-вектора (или вектора состояния) Луны относительно
    // барицентра Солнечной Системы.
    void calculateBaseMoon(unsigned resType, double jed, double* res) const;

}; // class EphemerisRelease

} // namespace dph

#endif // DEPHEM_H
