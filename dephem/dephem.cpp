#include "dephem.h"

#include <cmath>

dph::DevelopmentEphemeris::DevelopmentEphemeris()
{
    clear();
}

dph::DevelopmentEphemeris::DevelopmentEphemeris(const std::string& filePath)
{
    open(filePath);
}

dph::DevelopmentEphemeris::DevelopmentEphemeris(const DevelopmentEphemeris& other)
{
    copyHere(other);
}

dph::DevelopmentEphemeris& dph::DevelopmentEphemeris::operator=(const
    DevelopmentEphemeris& other)
{
    copyHere(other);
    return *this;
}

bool dph::DevelopmentEphemeris::open(const std::string& filePath)
{
    clear();

    m_file.open(filePath, std::ios::binary);

    if (!m_file.is_open())
        return false;

    else if (!read())
    {
        m_file.close();
        clear();
        return false;
    }

    m_filePath = filePath;
    return true;
}

// Положение target относительно center на момент времени jed.
bool dph::DevelopmentEphemeris::bodyPosition(int target, int center, double jed,
    double pos[3]) const
{
    return body(0, jed, target, center, pos);
}

// Положение и скорость target относительно center на момент времени jed.
bool dph::DevelopmentEphemeris::bodyState(int target, int center, double jed,
    double state[6]) const
{
    return body(1, jed, target, center, state);
}

// Значение отдельного элемента item на момент времени jed.
bool dph::DevelopmentEphemeris::item(int item, double jed, double* result) const
{
    if (item < 0 || item > 14)
        return false;
    else if (jed < m_beginJed || jed > m_endJed)
        return false;

    return baseItem(9, jed, item, result);
}


bool dph::DevelopmentEphemeris::body(int resType, double jed,
    int target, int center, double* res) const
{
    // Допустимые значения параметров:
    // -------------------------------
    // - resType:
    //   0 - Получить значение радиус-вектора,
    //   1 - Получить значение вектора состояния
    //   Примечание: используй значения из dph::Calculate.
    //
    // - jed:
    //   jed должен принадлежать промежутку: [m_startDate : m_endDate].
    //
    // - target, center:
    //   ------------------------------------
    //   Индекс Название
    //   ------------------------------------
    //   1      Меркурий
    //   2      Венера
    //   3      Земля
    //   4      Марс
    //   5      Юпитер
    //   6      Сатурн
    //   7      Уран
    //   8      Нептун
    //   9      Плутон
    //   10     Луна
    //   11     Солнце
    //   12     Барицентр Солнечной Системы
    //   13     Барицентр системы Земля-Луна
    //   ------------------------------------
    //   Примечание: используй значения из dph::Body.
    //
    // - res:
    //   От пользователя требуется знать, каков минимальный размер массива
    //   для выбранного результата вычислений. Не должен быть нулевым
    //   указателем.

    if (resType > 1)
    {
        return false;
    }
    else if (target == 0 || center == 0)
    {
        return false;
    }
    else if (target > 13 || center > 13)
    {
        return false;
    }
    else if (jed < m_beginJed || jed > m_endJed)
    {
        return false;
    }

    // Количество требуемых компонент.
    int componentsCount = resType == 0 ? 3 : 6;

    // Выбор методики вычисления в зависимости от комбинации искомого и
    // центрального тела.
    if (target == center)
    {
        // Случай 1: искомое тело совападает с центральным.

        memset(res, 0, sizeof(double) * componentsCount);
    }
    else if (target == B_SSBARY || center == B_SSBARY)
    {
        // Случай 2: искомым или центральным телом является барицентр CC.

        // Индекс тела, что не является барицентром СС.
        int notSSBARY = target == B_SSBARY ? center : target;

        // Выбор метода вычисления в зависимости от тела.
        bool ok = false;
        switch (notSSBARY) {
        case B_EARTH:  ok = ssbaryEarth(resType, jed, res);   break;
        case B_MOON:   ok = ssbaryMoon(resType, jed, res);    break;
        case B_EMBARY: ok = baseItem(resType, jed, 2, res); break;
        default:       ok = baseItem(resType, jed, notSSBARY - 1, res);
        }

        if (!ok)
            return false;

        // Если барицентр СС является искомым телом, то возвращается
        // "зеркальный" вектор.
        if (target == B_SSBARY)
            for (int i = 0; i < componentsCount; ++i)
                res[i] = -res[i];
    }
    else if (target * center == 30 && target + center == 13)
    {
        // Случай 3: Искомым и центральным телами являетса Земля и Луна.

        // Луна относительно Земли.
        if (!baseItem(resType, jed, 9, res))
            return false;

        // Если искомым телом является Земля, то возвращается "зеркальный"
        // вектор.
        if (target == B_EARTH)
            for (int i = 0; i < componentsCount; ++i)
                res[i] = -res[i];
    }
    else
    {
        // Случай 4: Все остальные комбинации тел.
        // Сначала вычисляется значение центрального тела относительно
        // барицентра СС, после - искомого. Результатом является разница между
        // вектором центрального тела и искомого.

        // Массив для центрального тела.
        double centerBody[6];

        // Две итерации:
        for (int i = 0; i <= 1; ++i)
        {
            // Определение индекса и массива в зависимости от номера итерации.
            // i == 0 : работа с центральным телом.
            // i == 1 : работа с искомым телом.
            int bodyIndex = i == 0 ? center : target;
            double* arr = i == 0 ? centerBody : res;

            // Выбор метода вычисления в зависимости от тела.
            bool ok = false;
            switch (bodyIndex) {
            case B_EARTH:  ok = ssbaryEarth(resType, jed, arr);   break;
            case B_MOON:   ok = ssbaryMoon(resType, jed, arr);    break;
            case B_EMBARY: ok = baseItem(resType, jed, 2, arr); break;
            default:       ok = baseItem(resType, jed, bodyIndex - 1, arr);
            }

            if (!ok)
                return false;
        }

        // Разница между вектором центрального и искомого тела.
        for (int i = 0; i < componentsCount; ++i)
            res[i] -= centerBody[i];
    }

    return true;
}

bool dph::DevelopmentEphemeris::isOpen() const
{
    return m_file.is_open();
}

double dph::DevelopmentEphemeris::beginJed() const
{
    return m_beginJed;
}

double dph::DevelopmentEphemeris::endJed() const
{
    return m_endJed;
}

int dph::DevelopmentEphemeris::index() const
{
    return m_index;
}

std::string dph::DevelopmentEphemeris::label() const
{
    return m_label;
}

double dph::DevelopmentEphemeris::constant(const std::string& name, bool* ok) const
{
    auto found = m_constants.find(name);

    if (found != m_constants.end())
    {
        if (ok)
            *ok = true;
        return found->second;
    }
    else
    {
        if (ok)
            *ok = false;
        return 0;
    }
}

std::string dph::DevelopmentEphemeris::filePath() const
{
    return m_filePath;
}

std::vector<dph::Item> dph::DevelopmentEphemeris::itemList() const
{
    std::vector<dph::Item> itemList;

    for (int i = 0; i < 15; ++i)
        if (m_keys[i][1] != 0)
            itemList.push_back(Item(i));

    return itemList;
}

std::string dph::DevelopmentEphemeris::cutBackSpaces(const char* s, size_t size)
{
    for (size_t i = size - 1; i > 0; --i)
        if (s[i] == ' ' && s[i - 1] != ' ')
            return std::string(s, i);

    return std::string(s, size);
}

void dph::DevelopmentEphemeris::clear()
{
    m_filePath.clear();
    m_file.close();

    m_label.clear();
    m_index = 0;
    m_beginJed = 0.0;
    m_endJed = 0.0;
    m_blockSpan = 0.0;
    std::memset(m_keys, 0, sizeof(m_keys));
    m_au = 0.0;
    m_emrat = 0.0;
    std::map<std::string, double>().swap(m_constants); // SWAP TRICK

    m_ncoeff = 0;

    std::vector<double>().swap(m_buffer); // SWAP TRICK
}

void dph::DevelopmentEphemeris::copyHere(const DevelopmentEphemeris& other)
{
    // Используется в:
    // - Конструктор копирования.
    // - Оператор копирования.

    m_filePath = other.m_filePath;

    m_file.close();
    m_file.open(other.m_filePath.c_str(), std::ios::binary);

    m_label = other.m_label;
    m_index = other.m_index;
    m_beginJed = other.m_beginJed;
    m_endJed = other.m_endJed;
    m_blockSpan = other.m_blockSpan;
    std::memcpy(m_keys, other.m_keys, sizeof(m_keys));
    m_au = other.m_au;
    m_emrat = other.m_emrat;
    m_constants = other.m_constants;

    m_ncoeff = other.m_ncoeff;

    m_buffer = other.m_buffer;
}

bool dph::DevelopmentEphemeris::read()
{
    // Перейти в начало файла.
    m_file.seekg(0, std::ios::beg);

    // Подпись выпуска.
    char label[LABELS_COUNT][LABEL_SIZE];
    if (!m_file.read((char*)&label, sizeof(label)))
        return false;

    // Имена констант.
    char constNames[CCOUNT_MAX_NEW][CNAME_SIZE];
    if (!m_file.read((char*)&constNames, CNAME_SIZE * CCOUNT_MAX_OLD))
        return false;

    // Первая доступная дата.
    if (!m_file.read((char*)&m_beginJed, 8))
        return false;
    else if (std::isnan(m_beginJed) || std::isinf(m_beginJed))
        return false;

    // Последная доступная дата.
    if (!m_file.read((char*)&m_endJed, 8))
        return false;
    else if (std::isnan(m_endJed) || std::isinf(m_endJed))
        return false;
    else if (m_beginJed >= m_endJed)
        return false;

    // Временной интервал блока.
    if (!m_file.read((char*)&m_blockSpan, 8))
        return false;
    else if (std::isnan(m_blockSpan) || std::isinf(m_blockSpan))
        return false;
    else if (m_blockSpan == 0)
        return false;
    else if ((m_endJed - m_beginJed) < m_blockSpan)
        return false;

    // Количество констант.
    int32_t constCount;
    if (!m_file.read((char*)&constCount, 4))
        return false;
    else if (constCount < 0 || size_t(constCount) > CCOUNT_MAX_NEW)
        return false;

    // Астрономическая единица.
    if (!m_file.read((char*)&m_au, 8))
        return false;
    else if (std::isnan(m_au) || std::isinf(m_au))
        return false;
    else if (m_au == 0)
        return false;

    // Отношение массы Земли к массе Луны.
    if (!m_file.read((char*)&m_emrat, 8))
        return false;
    else if (std::isnan(m_emrat) || std::isinf(m_emrat))
        return false;
    else if (m_emrat == 0)
        return false;

    // Первые 12 ключей.
    int32_t keys[15][3];
    if (!m_file.read((char*)&keys, (12 * 3) * 4))
        return false;

    // Индекс выпуска.
    int32_t index;
    if (!m_file.read((char*)&index, 4))
        return false;
    m_index = static_cast<int>(index);

    // 13-й ключ.
    if (!m_file.read((char*)&keys[12], (3) * 4))
        return false;

    // Имена доп. констант (если есть).
    if (size_t(constCount) > CCOUNT_MAX_OLD)
    {
        // Количество дополнительных констант.
        int count = constCount - CCOUNT_MAX_OLD;

        auto ptr = &constNames[CCOUNT_MAX_OLD];

        if (!m_file.read((char*)&ptr, count * CNAME_SIZE))
            return false;
    }

    // 14-е и 15-е ключи.
    if (!m_file.read((char*)&keys[13], (3 * 2) * 4))
        return false;

    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 3; ++j)
            m_keys[i][j] = static_cast<int>(keys[i][j]);

    // Количество коэффициентов в блоке.
    m_ncoeff = 2;
    for (int i = 0; i < 15; ++i)
    {
        // Количество компонент для выбранного элемента.
        int comp = i == 11 ? 2 : i == 14 ? 1 : 3;
        m_ncoeff += comp * m_keys[i][1] * m_keys[i][2];
    }

    if (m_ncoeff <= 0)
        return false;

    // Переход к блоку с константами и их чтение.
    double constValues[CCOUNT_MAX_NEW];
    if (size_t(constCount) <= CCOUNT_MAX_NEW)
    {
        if (!m_file.seekg(m_ncoeff * 8, std::ios::beg))
            return false;

        if (!m_file.read((char*)&constValues, constCount * 8))
            return false;
    }

    // --| Форматирование |-------------------------------------------------- //

    // Формирование строк общей информации о выпуске.
    for (size_t i = 0; i < LABELS_COUNT; ++i)
    {
        m_label += cutBackSpaces(label[i], LABEL_SIZE);
        m_label += '\n';
    }

    // Заполнение контейнера m_constants именами и значениями констант.
    for (int i = 0; i < constCount; ++i)
    {
        std::string name = cutBackSpaces(constNames[i], CNAME_SIZE);
        m_constants[name] = constValues[i];
    }

    // --| Дополнительные вычисления |--------------------------------------- //

    // Изменить размер буфера для коэффициентов.
    m_buffer.resize(m_ncoeff);

    return true;
}

bool dph::DevelopmentEphemeris::fillBuffer(size_t blockNum) const
{
    size_t blockSize = m_ncoeff * 8;
    size_t adress = (2 + blockNum) * blockSize;

    if (!m_file.seekg(adress, std::ios::beg))
        return false;

    if (!m_file.read((char*)&m_buffer[0], blockSize))
        return false;

    return true;
}

// Базовый элемент.
bool dph::DevelopmentEphemeris::baseItem(int resType, double jed, int baseItem,
    double* res) const
{
    // Допустимые значения переданных параметров:
    // [1] baseItem - Индекс базового элемента выпуска (от нуля).
    //
    //     Нумерация базовых элементов выпуска
    //     -------------------------------------------------------------------
    //     Индекс Наименование
    //     -------------------------------------------------------------------
    //     0      Mercury
    //     1      Venus
    //     2      Earth-Moon barycenter
    //     3      Mars
    //     4      Jupiter
    //     5      Saturn
    //     6      Uranus
    //     7      Neptune
    //     8      Pluto
    //     9      Moon (geocentric)
    //     10     Sun
    //     11     Earth Nutations in longitude and obliquity (IAU 1980 model)
    //     12     Lunar mantle libration
    //     13     Lunar mantle angular velocity
    //     14     TT-TDB (at geocenter)
    //     -------------------------------------------------------------------
    // [2] jed - момент времени на который требуется получить требуемые
    //           значения.
    // [3] resType - индекс результата вычисления (см. dph::Calculate).
    // [4] res - указатель на массив для результата вычислений.

    // Внимание!
    // В ходе выполнения функции смысл переменных "normalizedTime" и "offset"
    // будет меняться.

    // Норм. время относительно всех блоков в выпуске.
    double normalizedTime = (jed - m_beginJed) / m_blockSpan;

    // Порядковый номер блока, соотв. заданной дате jed (целая часть от
    // normalizedTime).
    size_t offset = static_cast<size_t>(normalizedTime);

    // Заполнение буффера коэффициентами требуемого блока.
    // Если требуемый блок уже в кэше объекта, то он не заполняется повторно.
    // m_buffer[0] - дата начала блока.
    // m_buffer[1] - дата окончания блока.
    if (jed < m_buffer[0] || jed >= m_buffer[1])
    {
        // Если JED равна последней доступоной дате для вычислений, то
        // заполняется последний блок.
        size_t blockNum = offset - (jed == m_endJed ? 1 : 0);
        if (!fillBuffer(blockNum))
            return false;
    }

    if (jed == m_endJed)
    {
        // Порядковый номер подблока (последний подблок).
        offset = m_keys[baseItem][2] - 1;

        // Норм. время относительно подблока (в диапазоне от -1 до 1).
        normalizedTime = 1;
    }
    else
    {
        // Норм. время относительно всех подблоков.
        normalizedTime = (normalizedTime - offset) * m_keys[baseItem][2];

        // Порядковый номер подблока (целая часть от normalizedTime).
        offset = static_cast<size_t>(normalizedTime);

        // Норм. время относительно подблока (в диапазоне от -1 до 1).
        normalizedTime = 2 * (normalizedTime - offset) - 1;
    }

    // Количество компонент для выбранного базового элемента.
    int componentsCount = baseItem == 11 ? 2 : baseItem == 14 ? 1 : 3;

    // Кол-во коэффициентов на компоненту.
    int cpec = m_keys[baseItem][1];

    // Порядковый номер первого коэффициента в блоке.
    int coeff_pos = m_keys[baseItem][0] - 1 + componentsCount * offset * cpec;

    // Указатель на массив коэффициентов.
    const double* coeffs = &m_buffer[coeff_pos];

    // Обнуление массива результата вычислений.
    if (resType == 0)
    {
        memset(res, 0, sizeof(double) * componentsCount);
    }
    else
    {
        memset(res, 0, sizeof(double) * componentsCount * 2);
    }

    // 1. Оригинальное значение элемента.

    // Заполнение полиномов.
    double poly[MAX_POLY_SIZE];
    poly[0] = 1;
    poly[1] = normalizedTime;

    for (int i = 2; i < cpec; ++i)
        poly[i] = 2 * normalizedTime * poly[i - 1] - poly[i - 2];

    // Массив с оригинальным значением.
    for (int i = 0; i < componentsCount; ++i)
        for (int j = 0; j < cpec; ++j)
            res[i] += poly[j] * coeffs[i * cpec + j];

    // 2. Первая производная элемента.
    if (resType == 1)
    {
        double dpoly[MAX_POLY_SIZE];
        dpoly[0] = 0;
        dpoly[1] = 1;
        dpoly[2] = 4 * normalizedTime;

        // Заполнение полиномов.
        for (int i = 3; i < cpec; ++i)
            dpoly[i] = 2 * poly[i - 1] + 2 * normalizedTime * dpoly[i - 1] -
                dpoly[i - 2];

        // Коэффициент размерности для 1-й производной.
        double dunits = double(m_keys[baseItem][2]) / (43200 * m_blockSpan);

        // Вычисление координат.
        for (int i = 0; i < componentsCount; ++i)
        {
            for (int j = 0; j < cpec; ++j)
                res[i + componentsCount] += dpoly[j] * coeffs[i * cpec + j];

            res[i + componentsCount] *= dunits;
        }
    }

    return true;
}

// Земля относительно барицентра Солнечной Системы.
bool dph::DevelopmentEphemeris::ssbaryEarth(int resType, double jed,
    double* res) const
{
    // Барицентр сиситемы Земля-Луна относительно барицентра Солнечной Системы.
    if (!baseItem(resType, jed, 2, res))
        return false;

    // Луна относитльно Земли.
    double moonRelEarth[6];
    if (!baseItem(resType, jed, 9, moonRelEarth))
        return false;

    // Количество компонент.
    int componentsCount = resType == 0 ? 3 : 6;

    // Земля относительно барицентра Солнечной Системы.
    for (int i = 0; i < componentsCount; ++i)
        res[i] -= moonRelEarth[i] / (1.0 + m_emrat);

    return true;
}

// Луна относительно барицентра Солнечной Системы.
bool dph::DevelopmentEphemeris::ssbaryMoon(int resType, double jed, double* res) const
{
    // Барицентр сиситемы Земля-Луна относительно барицентра Солнечной Системы.
    if (!baseItem(resType, jed, 2, res))
        return false;

    // Луна относитльно Земли.
    double moonRelEarth[6];
    if (!baseItem(resType, jed, 9, moonRelEarth))
        return false;

    // Количество компонент.
    int componentsCount = resType == 0 ? 3 : 6;

    // Луна относительно барицентра Солнечной Системы.
    for (int i = 0; i < componentsCount; ++i)
        res[i] += moonRelEarth[i] * (m_emrat / (1.0 + m_emrat));

    return true;
}
