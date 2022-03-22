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

/**
 *
 * @brief Представление бинарного файла DE-эфемерид NASA JPL.
 *
 * Позволяет получить доступ к данным, хранимым в файле.
 */
class DevelopmentEphemeris
{
public:

    /// Конструктор по умолчанию.
    DevelopmentEphemeris();

    /**
     * @brief Конструктор по пути к бинарному файлу эфемерид.
     *
     * Создаёт новый объект эфемерид по файлу `filePath`.
     *
     * Успешность открытия можно проверить методом isReady().
     */
    DevelopmentEphemeris(const std::string& filePath);

    /// Конструктор копирования.
    DevelopmentEphemeris(const DevelopmentEphemeris& other);

    /// Оператор копирования.
    DevelopmentEphemeris& operator=(const DevelopmentEphemeris& other);

    /**
     * @brief Открыть файл эфемерид.
     *
     * Инициализирует объект эфемерид файлом `filePath`.
     *
     * @return `true`, при успешном открытии, иначе - `false`.
     */
    bool open(const std::string& filePath);

    /**
     * @brief Положение небесного тела.
     *
     * Положение `pos` (x, y, z, км) тела `target` относительно `center` на
     * момент времени `jed`.
     *
     * @return `true`, при успешном выполнении, иначе - `false`:
     *  - объект класса не инициализирован файлом эфемерид;
     *  - передан неверный индекс центрального или небесного тела;
     *  - передано неверное время `jed`.
     *
     * @note `jed` должен удовлетворять интервалу [beginJed(), endJed()].
     *
     * @warning
     * 1. метод не является потокобезопасным;
     * 2. указатель `pos` не проверяется на `nullptr`.
     */
    bool bodyPosition(int target, int center, double jed, double pos[3]) const;

    /**
     * @brief Положение и скорость небесного тела.
     *
     * Положение и скорость `state` (x, y, z, vx, vy, vz, км, км/с) тела
     * `target` относительно `center` на момент времени `jed`.
     *
     * @return `true`, при успешном выполнении, иначе - `false`:
     *  - объект класса не инициализирован файлом эфемерид;
     *  - передан неверный индекс центрального или небесного тела;
     *  - передано неверное время `jed`.
     *
     * @note `jed` должен удовлетворять интервалу [beginJed(), endJed()].
     *
     * @warning
     * 1. метод не является потокобезопасным;
     * 2. указатель `state` не проверяется на `nullptr`.
     */
    bool bodyState(int target, int center, double jed, double state[6]) const;

    /**
     * @brief Отдельный элемент выпуска эфемерид.
     *
     * Значение `result` элемента `item` на момент времени `jed`.
     *
     * @return `true`, при успешном выполнении, иначе - `false`:
     *  - объект класса не инициализирован файлом эфемерид;
     *  - передан неверный индекс центрального или небесного тела;
     *  - передано неверное время `jed`.
     *
     * @note `jed` должен удовлетворять интервалу [beginJed(), endJed()].
     *
     * @warning
     * 1. метод не является потокобезопасным;
     * 2. указатель `result` не проверяется на `nullptr`.
     */
    bool item(int item, double jed, double* result) const;

    /// @return `true`, если эфемериды открыты успешно, иначе - `false`.
    bool isOpen() const;

    /// @return начало интервала, доступного для расчёта.
    double beginJed() const;

    /// @return конец интервала, доступного для расчёта.
    double endJed() const;

    /// @return индекс выпуска эфемерид.
    int index() const;

    /// @return подпись выпуска эфемерид.
    std::string label() const;

    /// @return значение константы `name`.
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
