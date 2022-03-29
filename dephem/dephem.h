#ifndef DEPHEM_H
#define DEPHEM_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <fstream>
#include <string>
#include <vector>

/// @brief Пространство имён библиотеки dephem.
namespace dph {

/**
 * @brief Индексы небесных тел.
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
 * @see
 *  DevelopmentEphemeris::itemBase
 *  DevelopmentEphemeris::itemDerivative
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

/// @brief Оператор вывода обозначения `Item` в поток.
std::ostream& operator<<(std::ostream& out, Item item);

/**
 * @brief Представление бинарного файла эфемерид.
 * @details Позволяет получить доступ к данным, хранимым в файле.
 */
class DevelopmentEphemeris
{
public:
    /**
     * @return количество оригинальных компонент для элемента `item`.
     * @see dph::Item
     */
    static int itemSize(int item);

    /// @details Создаёт пустой объет.
    DevelopmentEphemeris();

    /**
     * Создаёт объект и открывает файл эфемерид по пути `filePath`.
     * @see isOpen()
     */
    DevelopmentEphemeris(const std::string& filePath);

    /// @details Создаёт копию объекта.
    DevelopmentEphemeris(const DevelopmentEphemeris& other);

    /// @details Копирует объект.
    DevelopmentEphemeris& operator=(const DevelopmentEphemeris& other);

    /**
     * Открывает файл эфемерид по пути `filePath`.
     * @return `true`, при успешном открытии, иначе - `false`.
     */
    bool open(const std::string& filePath);

    /// @details Закрывает файл эфемерид.
    void close();

    /**
     * Положение тела `target` относительно `center` на момент времени `jed`.\n
     * Результат записывается в массив `pos` (x, y, z, км).
     * @return `true`, при успешном выполнении, иначе - `false`.
     * @see dph::Body.
     */
    bool bodyPosition(Body target, Body center, double jed, double pos[3]);

    /**
     * Положение и скорость тела `target` относительно `center` на момент
     * времени `jed`.\n
     * Результат записывается в массив `state` (x, y, z, vx, vy, vz, км, км/с).
     * @return `true`, при успешном выполнении, иначе - `false`.
     * @see dph::Body.
     */
    bool bodyState(Body target, Body center, double jed, double state[6]);

    /**
     * Значение `item` на момент времени `jed`.\n
     * Результат записывается в массив `res`.
     * @return `true`, при успешном выполнении, иначе - `false`.
     * @see dph::Item.
     */
    bool item(Item item, double jed, double* res);

    /**
     * Значение `item` и его производной на момент времени `jed`.\n
     * Результат записывается в массив `res`.
     * @return `true`, при успешном выполнении, иначе - `false`.
     * @see dph::Item.
     */
    bool item2(Item item, double jed, double* res);

    /// @return `true`, если файл эфемерид открыт, иначе - `false`.
    bool isOpen() const;

    /// @return начало доступного интервала.
    double beginJed() const;

    /// @return конец доступного интервала.
    double endJed() const;

    /// @return номер выпуска эфемерид.
    int index() const;

    /// @return подпись выпуска эфемерид.
    std::string label() const;

    /// @return значение константы `name`.
    double constant(const std::string& name, bool* ok = nullptr) const;

    /// @return список констант выпуска эфемерид.
    std::vector<std::pair<std::string, double>> constants() const;

    /// @return путь к открытому файлу эфемерид.
    std::string filePath() const;

    /**
     * @return список доступных элементов.
     * @see dph::Item
     */
    std::vector<Item> itemList() const;

    /**
     * @return `true`, если элемент есть в файле эфемерид, иначе - `false`.
     * @see dph::Item
     */
    bool hasItem(Item item) const;

private:
    // Формат эфемерид.
    static const size_t LABELS_COUNT = 3;      // Строк в подписи.
    static const size_t LABEL_SIZE = 84;       // Символов в строке подписи.
    static const size_t CNAME_SIZE = 6;        // Символов в имени константы.
    static const size_t CCOUNT_MAX_OLD = 400;  // Кол-во констант (ст. формат).
    static const size_t CCOUNT_MAX_NEW = 1000; // Кол-во констант (нов. формат).
    static const size_t MAX_POLY_SIZE = 15;    // Макс. размер полиномов.

    // Ключ поиска коэффициентов элемента.
    struct ItemKey
    {
        int offset = 0; // Положение относительно начала блока.
        int cpec = 0;   // Коэффициентов на одну компоненту.
        int span = 0;   // Интервал интерполяции.

        // Признак пустого ключа.
        bool empty() const;
    };

    // Работа с файлом
    std::string m_filePath;       // Путь к файлу эфемерид.
    std::ifstream m_file;         // Поток чтения файла.
    std::vector<double> m_buffer; // Буффер блока с коэффициентами.

    // Параметры эфемерид.
    std::string m_label; // Строковая информация о выпуске.
    int m_index;         // Номерная часть индекса выпуска.
    double m_beginJed;   // Дата начала выпуска (JED).
    double m_endJed;     // Дата окончания выпуска (JED).
    double m_blockSpan;  // Временная протяжённость блока.
    ItemKey m_keys[15];  // Ключи поиска коэффициентов элементов.
    double m_au;         // Астрономическая единица (км).
    double m_emrat;      // Отношение массы Земли к массе Луны.
    int m_ncoeff;        // Количество коэффициентов в блоке.

    // Константы выпуска в порядке их хранения в файле.
    std::vector<std::pair<std::string, double>> m_constants;

    // Обрезать повторяющиеся пробелы (' ') с конца массива символов "s"
    // размера "arraySize".
    static std::string cutBackSpaces(const char* s, size_t size);

    // Приведение объекта к изначальному состоянию.
    void clear();

    // Копирование информации из объекта "other" в текущий объект.
    void copy(const DevelopmentEphemeris& other);

    // Чтение файла.
    bool read();

    // Заполнение буффера "m_buffer" коэффициентами требуемого блока.
    bool fillBuffer(size_t blockNum);

    // Положение/Состояние тела.
    bool body(Body target, Body center, double jed, int resType, double* res);

    // Значение элемента.
    bool item(int item, double jed, int resType, double* res);

    // Земля относительно барицентра Солнечной Системы.
    bool ssbaryEarth(double jed, int resType, double* res);

    // Луна относительно барицентра Солнечной Системы.
    bool ssbaryMoon(double jed, int resType, double* res);

}; // class EphemerisRelease

} // namespace dph

#endif // DEPHEM_H
