#include "dephem.h"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace {

std::string __enum2str(dph::Body body)
{
    using namespace dph;

    switch (body) {
    case B_MERCURY: return "B_MERCURY";
    case B_VENUS:   return "B_VENUS";
    case B_EARTH:   return "B_EARTH";
    case B_MARS:    return "B_MARS";
    case B_JUPITER: return "B_JUPITER";
    case B_SATURN:  return "B_SATURN";
    case B_URANUS:  return "B_URANUS";
    case B_NEPTUNE: return "B_NEPTUNE";
    case B_PLUTO:   return "B_PLUTO";
    case B_MOON:    return "B_MOON";
    case B_SUN:     return "B_SUN";
    case B_SSBARY:  return "B_SSBARY";
    case B_EMBARY:  return "B_EMBARY";
    }

    return "unknown";
}

std::string __enum2str(dph::Item item)
{
    using namespace dph;

    switch (item) {
    case I_MERCURY: return "I_MERCURY";
    case I_VENUS:   return "I_VENUS  ";
    case I_EMBARY:  return "I_EMBARY ";
    case I_MARS:    return "I_MARS   ";
    case I_JUPITER: return "I_JUPITER";
    case I_SATURN:  return "I_SATURN ";
    case I_URANUS:  return "I_URANUS ";
    case I_NEPTUNE: return "I_NEPTUNE";
    case I_PLUTO:   return "I_PLUTO  ";
    case I_MOON:    return "I_MOON   ";
    case I_SUN:     return "I_SUN    ";
    case I_EN:      return "I_EN     ";
    case I_LML:     return "I_LML    ";
    case I_LMAV:    return "I_LMAV   ";
    case I_TTMTDB:  return "I_TTMTDB ";
    }

    return "unknown";
}

}

/// @brief Оператор вывода обозначения `Item` в поток.
std::ostream& dph::operator<<(std::ostream& out, Body body)
{
    return out << __enum2str(body);
}

std::ostream& dph::operator<<(std::ostream& out, Item item)
{
    return out << __enum2str(item);
}

std::ostream& dph::operator<<(std::ostream& out, const std::vector<Item>& items)
{
    if (items.empty())
    {
        out << "warning: item list is empty" << std::endl;
    }
    else
    {
        for (auto& item : items)
            out << item << std::endl;
    }

    return out;
}

std::ostream& dph::operator<<(std::ostream& out,
    const dph::DevelopmentEphemeris::Constant& c)
{
    out << std::setw(8) << std::left << c.name + ": " << c.value;
    return out;
}

std::ostream& dph::operator<<(std::ostream& out,
    const std::vector<DevelopmentEphemeris::Constant>& constants)
{
    if (constants.empty())
    {
        out << "warning: constant list is empty" << std::endl;
    }
    else
    {
        for (auto& c : constants)
            out << c << std::endl;
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
    auto found = std::find_if(m_constants.begin(), m_constants.end(),
        [&](const Constant& c) { return c.name == name; });

    if (found != m_constants.end())
    {
        if (ok)
            *ok = true;

        return found->value;
    }
    else
    {
        if (ok)
            *ok = false;

        return 0;
    }
}

std::vector<dph::DevelopmentEphemeris::Constant>
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

dph::TestpoReport dph::DevelopmentEphemeris::testpoReport(
        const std::string& testpoFilePath)
{
    dph::TestpoReport report;

    if (!isOpen())
    {
        report.m_errmsg = "ephemeris object is not initialized";
        return report;
    }

    std::ifstream testpoFile(testpoFilePath);

    if (!testpoFile.is_open())
    {
        report.m_errmsg = "unable to open testpo file";
        return report;
    }

    std::string line;
    size_t lineCount = 0;

    while (std::getline(testpoFile, line))
    {
        lineCount++;
        report.m_header += line + "\n";

        if (line.find("EOT") != std::string::npos)
            break;
    }

    if (testpoFile.peek() == EOF)
    {
        report.m_errmsg = "unable to find EOT marker";
        return report;
    }

    while (std::getline(testpoFile, line))
    {
        lineCount++;

        int index;
        std::string date;
        double jed;
        int t, c, x;
        double coordinate;

        std::istringstream ss(line);

        if (ss >> index >> date >> jed >> t >> c >> x >> coordinate)
        {
            double result;
            int caseCode = testpoCase(jed, t, c, x, result);

            TestpoReport::TestCase _case;
            _case.line = line;
            _case.jed = jed;
            _case.t = t;
            _case.c = c;
            _case.x = x;
            _case.coordinate = coordinate;
            _case.result = result;
            _case.caseCode = caseCode;

            report.m_testCases.push_back(_case);
        }
        else
        {
            report.m_errmsg = "invalid format at line " +
                std::to_string(lineCount);

            return report;
        }
    }

    return report;
}

double dph::DevelopmentEphemeris::bodyGm(Body body, bool* ok)
{
    double c = m_au * m_au * m_au / 86400 / 86400;

    switch (body) {
    case B_MERCURY: return constant("GM1", ok) * c;
    case B_VENUS:   return constant("GM2", ok) * c;
    case B_EMBARY:  return constant("GMB", ok) * c;
    case B_MARS:    return constant("GM4", ok) * c;
    case B_JUPITER: return constant("GM5", ok) * c;
    case B_SATURN:  return constant("GM6", ok) * c;
    case B_URANUS:  return constant("GM7", ok) * c;
    case B_NEPTUNE: return constant("GM8", ok) * c;
    case B_PLUTO:   return constant("GM9", ok) * c;
    case B_SUN:     return constant("GMS", ok) * c;
    default: break;
    }

    if (body == B_MOON)
    {
        double embary = bodyGm(B_EMBARY, ok);

        if (ok && !*ok)
            return 0;

        return embary / (1 + m_emrat);
    }
    else if (body == B_EARTH)
    {
        double embary = bodyGm(B_EMBARY, ok);

        if (ok && !*ok)
            return 0;

        return (embary * m_emrat) / (1 + m_emrat);
    }

    if (ok)
        *ok = false;

    return 0;
}

std::vector<dph::DevelopmentEphemeris::Constant>
    dph::DevelopmentEphemeris::bodyGms()
{
    std::vector<Constant> data;

    auto bodies = { B_MERCURY, B_VENUS, B_EARTH, B_MARS, B_JUPITER, B_SATURN,
        B_URANUS, B_NEPTUNE, B_PLUTO, B_MOON, B_SUN, B_EMBARY };

    data.reserve(bodies.size());

    for (auto body : bodies)
    {
        Constant c;
        c.name = __enum2str(body);
        c.value = bodyGm(body);
        data.push_back(c);
    }

    return data;
}

int dph::DevelopmentEphemeris::makeBinary(double beginJed, double endJed,
    const std::string& filePath)
{
    if (!isOpen())
        return 1;

    if (beginJed < m_beginJed || endJed > m_endJed)
        return 2;

    std::ofstream file(filePath, std::ios::out | std::ios::binary);

    if (!file.is_open())
        return 3;

    size_t blockSize = m_ncoeff * 8;
    std::vector<char> buff(blockSize);

    if (!m_file.seekg(0, std::ios::beg))
        return 4;

    for (int i = 0; i < 2; ++i)
    {
        if (!m_file.read(buff.data(), blockSize))
            return 4;

        if (!file.write(buff.data(), blockSize))
            return 5;
    }

    size_t firstBlock = (beginJed - m_beginJed) / m_blockSpan;
    size_t lastBlock = (endJed - m_beginJed) / m_blockSpan;

    if (!m_file.seekg((2 + firstBlock) * blockSize), std::ios::beg)
        return 4;

    size_t blocksCount = lastBlock - firstBlock + 1;

    double realBeginJed = 0;
    double realEndJed = 0;

    for (size_t i = 0; i < blocksCount; ++i)
    {
        if (!m_file.read(buff.data(), blockSize))
            return 4;

        if (!file.write(buff.data(), blockSize))
            return 5;

        if (i == 0)
        {
            memcpy((char*)&realBeginJed, buff.data(), sizeof(double));
        }

        else if (i == blocksCount - 1)
        {
            memcpy((char*)&realEndJed, buff.data() + sizeof(double),
                   sizeof(double));
        }
    }

    // Вернуться и переписать даты начала и конца.
    size_t address = LABELS_COUNT * LABEL_SIZE + CNAME_SIZE * CCOUNT_MAX_OLD;
    if (!file.seekp(address, std::ios::beg))
        return 5;

    if (!file.write((char*)&realBeginJed, sizeof(double)))
        return 4;

    if (!file.write((char*)&realEndJed, sizeof(double)))
        return 4;

    return 0;
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
        Constant c;
        c.name = cutBackSpaces(constNames[i], CNAME_SIZE);
        c.value = constValues[i];
        m_constants.push_back(c);
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

    // 1. Оригинальное значение элемента.

    // Заполнение полиномов.
    double poly[MAX_POLY_SIZE];
    poly[0] = 1;
    poly[1] = normalizedTime;

    for (int i = 2; i < key.cpec; ++i)
        poly[i] = 2 * normalizedTime * poly[i - 1] - poly[i - 2];

    // Массив с оригинальным значением.
    for (int i = 0; i < componentsCount; ++i)
    {
        double sum = 0;

        for (int j = key.cpec - 1; j != -1 ; --j)
            sum += poly[j] * coeffs[i * key.cpec + j];

        res[i] = sum;
    }

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
            double sum = 0;

            for (int j = key.cpec - 1; j != -1; j--)
                sum += dpoly[j] * coeffs[i * key.cpec + j];

            res[i + componentsCount] = sum * dunits;
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

int dph::DevelopmentEphemeris::testpoCase(double jed, int t, int c, int x,
    double& result)
{
    if (jed < m_beginJed || jed > m_endJed)
        return 1;

    if (t < 1 || t > 17)
        return 2;

    if (t <= 13)
    {
        if (c < 1 || c > 13)
            return 3;

        if (x < 1 || x > 6)
            return 4;

        double res[6];
        if (!bodyState(Body(t), Body(c), jed, res))
            return 5;

        result = res[x - 1] / m_au;

        if (x > 3)
            result *= 86400;
    }
    else
    {
        if (c != 0)
            return 3;

        auto item = Item(t - 3);
        int size = itemSize(item);

        if (x - 1 >= size * 2)
            return 4;

        double res[6];
        if (!item2(item, jed, res))
            return 5;

        result = res[x - 1];

        if (x - 1 >= size)
            result *= 86400;
    }

    return 0;
}

double dph::TestpoReport::TestCase::error() const
{
    if (caseCode == 0)
        return fabs(coordinate - result);
    else
        return 0;
}

const std::vector<dph::TestpoReport::TestCase>&
    dph::TestpoReport::testCases() const
{
    return m_testCases;
}

std::vector<dph::TestpoReport::TestCase>
    dph::TestpoReport::warnings(double eps) const
{
    std::vector<TestCase> cases;

    for (const auto& _case : m_testCases)
        if (_case.error() > eps)
            cases.push_back(_case);

    return cases;
}

int dph::TestpoReport::write(std::ostream& os) const
{
    os << m_header;

    for (const auto& _case : m_testCases)
        os << _case << std::endl;

    return 0;
}

int dph::TestpoReport::write(const std::string& filePath) const
{
    std::ofstream file(filePath);

    if (!file.is_open())
        return 1;

    auto writeCode = write(file);

    if (writeCode)
        return writeCode + 1;

    return 0;
}

bool dph::TestpoReport::ok() const
{
    return m_errmsg.empty();
}

std::string dph::TestpoReport::errmsg() const
{
    return m_errmsg;
}

std::ostream& dph::operator<<(std::ostream& out,
    const dph::TestpoReport::TestCase& _case)
{
    using namespace std;

    out << _case.line
        << setw(30) << setprecision(20) << fixed      << _case.result
        << setw(8) << setprecision(1)   << scientific << _case.error()
        << " " << _case.caseCode;

    return out;
}
