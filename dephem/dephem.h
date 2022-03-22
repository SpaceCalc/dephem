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

/// @brief Пространство имён библиотеки dephem.
namespace dph {

/**
 * @brief Индексы небесных тел.
 *
 * @see
 *  DevelopmentEphemeris::bodyPosition
 *  DevelopmentEphemeris::bodyState
 */
enum Body
{
    B_MERCURY = 1,  ///< Меркурий
    B_VENUS   = 2,  ///< Венера
    B_EARTH   = 3,  ///< Земля
    B_MARS    = 4,  ///< Марс
    B_JUPITER = 5,  ///< Юпитер
    B_SATURN  = 6,  ///< Сатур
    B_URANUS  = 7,  ///< Уран
    B_NEPTUNE = 8,  ///< Нептун
    B_PLUTO   = 9,  ///< Плутон
    B_MOON    = 10, ///< Луна
    B_SUN     = 11, ///< Солнце
    B_SSBARY  = 12, ///< Барицентр Солнечной Системы
    B_EMBARY  = 13  ///< Барицентр системы Земля-Луна
};

/**
 * @brief Индексы элементов.
 *
 * @see
 *  DevelopmentEphemeris::item
 */
enum Item {
    I_MERCURY = 0,  ///< Меркурий
    I_VENUS   = 1,  ///< Венера
    I_EMBARY  = 2,  ///< Барицентр системы Земля-Луна
    I_MARS    = 3,  ///< Марс
    I_JUPITER = 4,  ///< Юпитер
    I_SATURN  = 5,  ///< Сатурн
    I_URANUS  = 6,  ///< Уран
    I_NEPTUNE = 7,  ///< Нептун
    I_PLUTO   = 8,  ///< Плутон
    I_MOON    = 9,  ///< Луна (относительно Земли)
    I_SUN     = 10, ///< Солнце
    I_EN      = 11, ///< Земные нутации в долготе и наклоне (модель IAU 1980)
    I_LML     = 12, ///< Либрации мантии Луны
    I_LMAV    = 13, ///< Угловая скорость мантии Луны
    I_TTMTDB  = 14  ///< TT-TDB (в геоцентре)
};

/**
 *
 * @brief Представление бинарного файла эфемерид.
 *
 * Позволяет получить доступ к данным, хранимым в файле.
 */
class DevelopmentEphemeris
{
public:

    /// @details Создаёт неинициализированный объет эфемерид.
    DevelopmentEphemeris();

    /**
     * Создаёт объект эфемерид по файлу `filePath`.
     *
     * Успешность открытия можно проверить методом isOpen().
     */
    DevelopmentEphemeris(const std::string& filePath);

    /// @details Конструктор копирования.
    DevelopmentEphemeris(const DevelopmentEphemeris& other);

    /// @details Оператор копирования.
    DevelopmentEphemeris& operator=(const DevelopmentEphemeris& other);

    /**
     * Инициализирует объект эфемерид файлом `filePath`.
     *
     * @return `true`, при успешном открытии, иначе - `false`.
     *
     * @see isOpen()
     */
    bool open(const std::string& filePath);

    /**
     * Положение тела `target` относительно `center` на момент времени `jed`.\n
     * Результат записывается в массив `pos` (x, y, z, км).
     *
     * @return `true`, при успешном выполнении, иначе - `false`.
     *
     * @code{.cpp}
     *  using namespace dph;
     *  DevelopmentEphemeris ephem("path/to/file");
     *  double moon[3];
     *  bool ok = ephem.bodyPosition(B_MOON, B_EARTH, 2451544.5, moon);
     * @endcode
     *
     * @note
     *  Чтобы не ошибиться с индексами тел используйте dph::Body.
     *
     * @see
     *  dph::Body
     */
    bool bodyPosition(int target, int center, double jed, double pos[3]) const;

    /**
     * Положение и скорость тела `target` относительно `center` на момент
     * времени `jed`.\n
     * Результат записывается в массив `state` (x, y, z, vx, vy, vz, км, км/с).
     *
     * @return `true`, при успешном выполнении, иначе - `false`.
     *
     * @code{.cpp}
     *  using namespace dph;
     *  DevelopmentEphemeris ephem("path/to/file");
     *  double moon[6];
     *  bool ok = ephem.bodyState(B_MOON, B_EARTH, 2451544.5, moon);
     * @endcode
     *
     * @note
     *  Чтобы не ошибиться с индексами тел используйте dph::Body.
     *
     * @see
     *  dph::Body
     */
    bool bodyState(int target, int center, double jed, double state[6]) const;

    /**
     * Значение элемента `item` на момент времени `jed`.\n
     * Результат записывается в массив `result`.
     *
     * @return `true`, при успешном выполнении, иначе - `false`.
     *
     * @code{.cpp}
     *  using namespace dph;
     *  DevelopmentEphemeris ephem("path/to/file");
     *  double earthNutations[2];
     *  bool ok = ephem.item(I_EN, 2451544.5, earthNutations);
     * @endcode
     *
     * @note
     *  Чтобы не ошибиться с индексом элемента используйте dph::Item.
     *
     * @see
     *  dph::Item
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

    /// @return путь к файлу эфемерид.
    std::string filePath() const;

    /**
     * @return список доступных элементов.
     * @see dph::Item
     */
    std::vector<Item> itemList() const;

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
