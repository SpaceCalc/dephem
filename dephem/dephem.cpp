#include "dephem.h"

#include <cmath>
#include <cstring>
#include <algorithm>

std::ostream& dph::operator<<(std::ostream& out, Item item)
{
    switch (item) {
    case I_MERCURY: out << "I_MERCURY"; break;
    case I_VENUS:   out << "I_VENUS  "; break;
    case I_EMBARY:  out << "I_EMBARY "; break;
    case I_MARS:    out << "I_MARS   "; break;
    case I_JUPITER: out << "I_JUPITER"; break;
    case I_SATURN:  out << "I_SATURN "; break;
    case I_URANUS:  out << "I_URANUS "; break;
    case I_NEPTUNE: out << "I_NEPTUNE"; break;
    case I_PLUTO:   out << "I_PLUTO  "; break;
    case I_MOON:    out << "I_MOON   "; break;
    case I_SUN:     out << "I_SUN    "; break;
    case I_EN:      out << "I_EN     "; break;
    case I_LML:     out << "I_LML    "; break;
    case I_LMAV:    out << "I_LMAV   "; break;
    case I_TTMTDB:  out << "I_TTMTDB "; break;
    default:        out << "unknown";
    }

    return out;
}

int dph::DevelopmentEphemeris::itemSize(Item item)
{
    if (item < 0 || item > 14)
        return -1;

    return (item == 14) ? 1 : (item == 11) ? 2 : 3;
}

bool dph::DevelopmentEphemeris::ItemKey::empty() const
{
    return cpec == 0 && span == 0;
}

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
    copy(other);
}

dph::DevelopmentEphemeris& dph::DevelopmentEphemeris::operator=(const
    DevelopmentEphemeris& other)
{
    copy(other);
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

void dph::DevelopmentEphemeris::close()
{
    m_file.close();
    clear();
}

// Положение target относительно center на момент времени jed.
bool dph::DevelopmentEphemeris::bodyPos(Body target, Body center,
    double jed, double pos[3])
{
    return body(target, center, jed, 0, pos);
}

// Положение и скорость target относительно center на момент времени jed.
bool dph::DevelopmentEphemeris::bodyState(Body target, Body center,
    double jed, double state[6])
{
    return body(target, center, jed, 1, state);
}

// Значение отдельного элемента item на момент времени jed.
bool dph::DevelopmentEphemeris::item(Item item, double jed, double* res)
{
    if (item < 0 || item > 14)
        return false;
    else if (jed < m_beginJed || jed > m_endJed)
        return false;

    return this->item(item, jed, 0, res);
}

bool dph::DevelopmentEphemeris::item2(Item item, double jed, double* res)
{
    if (item < 0 || item > 14)
        return false;
    else if (jed < m_beginJed || jed > m_endJed)
        return false;

    return this->item(item, jed, 1, res);
}

bool dph::DevelopmentEphemeris::body(Body target, Body center,
    double jed, int resType, double* res)
{
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
        case B_EARTH:  ok = ssbaryEarth(jed, resType, res); break;
        case B_MOON:   ok = ssbaryMoon(jed, resType, res); break;
        case B_EMBARY: ok = item(I_EMBARY, jed, resType, res); break;
        default:       ok = item(notSSBARY - 1, jed, resType, res);
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
        if (!item(I_MOON, jed, resType, res))
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
            case B_EARTH:  ok = ssbaryEarth(jed, resType, arr); break;
            case B_MOON:   ok = ssbaryMoon(jed, resType, arr); break;
            case B_EMBARY: ok = item(I_EMBARY, jed, resType, arr); break;
            default:       ok = item(bodyIndex - 1, jed, resType, arr);
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

double dph::DevelopmentEphemeris::constant(const std::string& name,
    bool* ok) const
{
    for (auto it = m_constants.begin(); it != m_constants.end(); it++)
    {
        if (it->first == name)
        {
            if (ok)
                *ok = true;
            return it->second;
        }
    }

    if (ok)
        *ok = false;
    return 0;
}

std::vector<std::pair<std::string, double>>
    dph::DevelopmentEphemeris::constants() const
{
    return m_constants;
}

std::string dph::DevelopmentEphemeris::filePath() const
{
    return m_filePath;
}

std::vector<dph::Item> dph::DevelopmentEphemeris::items() const
{
    std::vector<dph::Item> itemList;

    for (int i = 0; i < 15; ++i)
        if (!m_keys[i].empty())
            itemList.push_back(Item(i));

    return itemList;
}

bool dph::DevelopmentEphemeris::hasItem(Item item) const
{
    if (item < 0 || item > 14)
        return false;

    return !m_keys[item].empty();
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
    m_buffer.clear();

    m_label.clear();
    m_index = 0;
    m_beginJed = 0;
    m_endJed = 0;
    m_blockSpan = 0;
    std::memset(m_keys, 0, sizeof(m_keys));
    m_au = 0;
    m_emrat = 0;
    m_ncoeff = 0;
    m_constants.clear();
}

void dph::DevelopmentEphemeris::copy(const DevelopmentEphemeris& other)
{
    m_filePath = other.m_filePath;
    m_file.close();
    m_file.open(m_filePath, std::ios::binary);
    m_buffer = other.m_buffer;

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
    {
        m_keys[i].offset = keys[i][0];
        m_keys[i].cpec = keys[i][1];
        m_keys[i].span = keys[i][2];
    }

    // Количество коэффициентов в блоке.
    m_ncoeff = 2;
    for (int i = 0; i < 15; ++i)
        m_ncoeff += itemSize(Item(i)) * m_keys[i].cpec * m_keys[i].span;

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
    m_constants.reserve(constCount);
    for (int i = 0; i < constCount; ++i)
    {
        std::string name = cutBackSpaces(constNames[i], CNAME_SIZE);
        m_constants.push_back({name, constValues[i]});
    }

    // --| Дополнительные вычисления |--------------------------------------- //

    // Изменить размер буфера для коэффициентов.
    m_buffer.resize(m_ncoeff);

    return true;
}

bool dph::DevelopmentEphemeris::fillBuffer(size_t blockNum)
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
bool dph::DevelopmentEphemeris::item(int item, double jed, int resType,
    double* res)
{
    // Cмысл переменных normalizedTime и offset будет меняться.

    const ItemKey& key = m_keys[item];

    if (key.empty())
        return false;

    // Норм. время относительно всех блоков в выпуске.
    double normalizedTime = (jed - m_beginJed) / m_blockSpan;

    // Порядковый номер блока, соотв. заданной дате jed (целая часть от
    // normalizedTime).
    size_t offset = size_t(normalizedTime);

    // Заполнение буффера коэффициентами требуемого блока.
    // Если требуемый блок уже в кэше объекта, то он не заполняется повторно.
    // m_buffer[0] - дата начала блока.
    // m_buffer[1] - дата окончания блока.
    if (jed < m_buffer[0] || jed >= m_buffer[1])
    {
        // Если jed равна последней доступоной дате для вычислений, то
        // заполняется последний блок.
        size_t blockNum = offset - (jed == m_endJed ? 1 : 0);

        if (!fillBuffer(blockNum))
            return false;
    }

    if (jed == m_endJed)
    {
        // Порядковый номер подблока (последний подблок).
        offset = key.span - 1;

        // Норм. время относительно подблока (в диапазоне от -1 до 1).
        normalizedTime = 1;
    }
    else
    {
        // Норм. время относительно всех подблоков.
        normalizedTime = (normalizedTime - offset) * key.span;

        // Порядковый номер подблока (целая часть от normalizedTime).
        offset = size_t(normalizedTime);

        // Норм. время относительно подблока (в диапазоне от -1 до 1).
        normalizedTime = 2 * (normalizedTime - offset) - 1;
    }

    // Количество компонент для выбранного базового элемента.
    int componentsCount = itemSize(Item(item));

    // Порядковый номер первого коэффициента в блоке.
    int coeff_pos = key.offset - 1 + componentsCount * offset * key.cpec;

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

    for (int i = 2; i < key.cpec; ++i)
        poly[i] = 2 * normalizedTime * poly[i - 1] - poly[i - 2];

    // Массив с оригинальным значением.
    for (int i = 0; i < componentsCount; ++i)
        for (int j = 0; j < key.cpec; ++j)
            res[i] += poly[j] * coeffs[i * key.cpec + j];

    // 2. Первая производная элемента.
    if (resType == 1)
    {
        double dpoly[MAX_POLY_SIZE];
        dpoly[0] = 0;
        dpoly[1] = 1;
        dpoly[2] = 4 * normalizedTime;

        // Заполнение полиномов.
        for (int i = 3; i < key.cpec; ++i)
            dpoly[i] = 2 * poly[i - 1] + 2 * normalizedTime * dpoly[i - 1] -
                dpoly[i - 2];

        // Коэффициент размерности для 1-й производной.
        double dunits = double(key.span) / (43200 * m_blockSpan);

        // Вычисление координат.
        for (int i = 0; i < componentsCount; ++i)
        {
            for (int j = 0; j < key.cpec; ++j)
                res[i + componentsCount] += dpoly[j] * coeffs[i * key.cpec + j];

            res[i + componentsCount] *= dunits;
        }
    }

    return true;
}

// Земля относительно барицентра Солнечной Системы.
bool dph::DevelopmentEphemeris::ssbaryEarth(double jed, int resType,
    double* res)
{
    // Барицентр сиситемы Земля-Луна относительно барицентра Солнечной Системы.
    if (!item(I_EMBARY, jed, resType, res))
        return false;

    // Луна относитльно Земли.
    double moonRelEarth[6];
    if (!item(I_MOON, jed, resType, moonRelEarth))
        return false;

    // Количество компонент.
    int componentsCount = resType == 0 ? 3 : 6;

    // Земля относительно барицентра Солнечной Системы.
    for (int i = 0; i < componentsCount; ++i)
        res[i] -= moonRelEarth[i] / (1.0 + m_emrat);

    return true;
}

// Луна относительно барицентра Солнечной Системы.
bool dph::DevelopmentEphemeris::ssbaryMoon(double jed, int resType, double* res)
{
    // Барицентр сиситемы Земля-Луна относительно барицентра Солнечной Системы.
    if (!item(I_EMBARY, jed, resType, res))
        return false;

    // Луна относитльно Земли.
    double moonRelEarth[6];
    if (!item(I_MOON, jed, resType, moonRelEarth))
        return false;

    // Количество компонент.
    int componentsCount = resType == 0 ? 3 : 6;

    // Луна относительно барицентра Солнечной Системы.
    for (int i = 0; i < componentsCount; ++i)
        res[i] += moonRelEarth[i] * (m_emrat / (1.0 + m_emrat));

    return true;
}
