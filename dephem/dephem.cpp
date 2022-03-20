#include "dephem.h"

dph::EphemerisRelease::EphemerisRelease(const std::string& filePath)
{
    // Инициализация внутренних переменных:
    clear();

    // Копирование пути к файлу:
    m_filePath = filePath;

    // Открытие файла:
    m_file.open(m_filePath.c_str(), std::ios::binary);

    // Файл открыт?
    bool isFileOpen = m_file.is_open();

    if (isFileOpen)
    {
        readAndPackData();

        if (isDataCorrect())
        {
            m_ready = true;
        }
        else
        {
            clear();
        }
    }
    else
    {
        m_filePath.clear();
    }
}

dph::EphemerisRelease::EphemerisRelease(const EphemerisRelease& other)
{
    if (other.m_ready)
    {
        copyHere(other);

        if (isDataCorrect())
        {
            m_ready = true;
        }
        else
        {
            m_ready = false;

            clear();
        }
    }
}

dph::EphemerisRelease& dph::EphemerisRelease::operator=(const
    EphemerisRelease& other)
{
    if (other.m_ready)
    {
        clear();
        copyHere(other);

        if (isDataCorrect())
        {
            m_ready = true;
        }
        else
        {
            m_ready = false;

            clear();
        }
    }

    return *this;
}

dph::EphemerisRelease::~EphemerisRelease()
{
    m_file.close();
}

void dph::EphemerisRelease::calculateBody(unsigned resType, double jed,
    unsigned target, unsigned center, double* res) const
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


    //Условия недопустимые для данного метода:
    if (this->m_ready == false)
    {
        return;
    }
    else if (resType > 1)
    {
        return;
    }
    else if (target == 0 || center == 0)
    {
        return;
    }
    else if (target > 13 || center > 13)
    {
        return;
    }
    else if (jed < m_beginJed || jed > m_endJed)
    {
        return;
    }
    else if (res == NULL)
    {
        return;
    }

    // Количество требуемых компонент:
    unsigned componentsCount = resType == CALC_STATE ? 6 : 3;

    // Выбор методики вычисления в зависимости от комбинации искомого и
    // центрального тела:
    if (target == center)
    {
        // Случай #1 : Искомое тело является центральным.
        //
        // Результатом является нулевой вектор.

        // Заполнение массива нулями:
        std::memset(res, 0, sizeof(double) * componentsCount);
    }
    else if (target == B_SSBARY || center == B_SSBARY)
    {
        // Случай #2: искомым или центральным телом является барицентр Солнечной
        // Системы.
        //
        // Так как все методы calculateBase для тел возвращают вектор
        // относительно барцентра СС, то достаточно вычислить пололжение второго
        // тела. В случае, если искомым телом является сам барицентр СС, то
        // возвращается "зеркальный" вектор второго тела.

        // Индекс тела, что не является барицентром СС:
        unsigned notSSBARY = target == B_SSBARY ? center : target;

        // Выбор метода вычисления в зависимости от тела:
        switch (notSSBARY)
        {
        case B_EARTH: calculateBaseEarth(jed, resType, res); break;
        case B_MOON: calculateBaseMoon(jed, resType, res); break;
        case B_EMBARY: calculateBaseItem(resType, jed, 2, res); break;
        default: calculateBaseItem(resType, jed, notSSBARY - 1, res);
        }

        // Если барицентр СС является искомым телом, то возвращается
        // "зеркальный" вектор:
        if (target == B_SSBARY)
        {
            for (unsigned i = 0; i < componentsCount; ++i)
            {
                res[i] = -res[i];
            }
        }
    }
    else if (target * center == 30 && target + center == 13)
    {
        // Случай #3 : Искомым и центральным телами являетса Земля и Луна (или
        // Луна и Земля).
        //
        // В этом случае достаточно получить значение положения Луны
        // относительно Земли (базовый элемент #9 (от нуля). В случае, если
        // искомым телом является Земля, то возвращается "зеркальный вектор".

        // Получение радиус-вектора (или вектора состояния) Луны относительно
        // Земли:
        calculateBaseItem(resType, jed, 9, res);

        // Если искомым телом является Земля, то возвращается "зеркальный"
        // вектор.
        if (target == B_EARTH)
        {
            for (unsigned i = 0; i < componentsCount; ++i)
            {
                res[i] = -res[i];
            }
        }
    }
    else
    {
        // Случай #4 : Все остальные комбинации тел.
        //
        // Сначала вычисляется значение центрального тела относительно
        // барицентра СС, после - искомого. Результатом является разница между
        // вектором центрального тела и искомого.

        // Массив для центрального тела:
        double centerBodyArray[6];

        // Две итерации:
        for (unsigned i = 0; i <= 1; ++i)
        {
            // Определение индекса и массива в зависимости от номера итерации.
            // i == 0 : работа с центральным телом.
            // i == 1 : работа с искомым телом.
            unsigned currentBodyIndex = i == 0 ? center : target;
            double* currentArray = i == 0 ? centerBodyArray : res;

            // Выбор метода вычисления в зависимости от тела:
            switch (currentBodyIndex) {
            case B_EARTH:
                calculateBaseEarth(jed, resType, currentArray); break;
            case B_MOON:
                calculateBaseMoon(jed, resType, currentArray); break;
            case B_EMBARY:
                calculateBaseItem(resType, jed, 2, currentArray); break;
            default:
                calculateBaseItem(resType, jed, currentBodyIndex - 1,
                    currentArray);
            }
        }

        // Разница между вектором центрального и искомого тела:
        for (unsigned i = 0; i < componentsCount; ++i)
        {
            res[i] -= centerBodyArray[i];
        }
    }
}

void dph::EphemerisRelease::calculateOther(unsigned resType, double jed,
    unsigned item, double* res) const
{
    // Допустимые значения параметров:
    // -------------------------------
    // - resType:
    //   0 - Получить оригинальное значение,
    //   1 - Получить оригинальное значение и его (их) производные первого
    //       порядка.
    //   Примечание: используй значения из dph::Calculate.
    //
    // - jed:
    //   jed должен принадлежать промежутку: [m_startDate : m_endDate].
    //
    // - item:
    //   ----------------------------------------------------------------
    //   Индекс Название
    //   ----------------------------------------------------------------
    //   14     Земные нутации по долдготе и наклонению (модель IAU 1980)
    //   15     Либрации лунной мантии
    //   16     Угловые скорости лунной мантии
    //   17     TT - TDB (в центре Земли).
    //   ----------------------------------------------------------------
    //   Примечание: используй значения из dph::Other.
    //
    // - res:
    //   От пользователя требуется знать, каков минимальный размер массива для
    //   выбранного результата вычислений. Не должен быть нулевым указателем.

    //Условия недопустимые для данного метода:
    if (this->m_ready == false)
    {
        return;
    }
    else if (resType > 1)
    {
        return;
    }
    else if (item < 14 || item > 17)
    {
        return;
    }
    else if (jed < m_beginJed || jed > m_endJed)
    {
        return;
    }
    else if (res == NULL)
    {
        return;
    }
    else
    {
        calculateBaseItem(resType, jed, item - 3, res);
    }
}


bool dph::EphemerisRelease::isReady() const
{
    return m_ready;
}

double dph::EphemerisRelease::beginJed() const
{
    return m_beginJed;
}

double dph::EphemerisRelease::endJed() const
{
    return m_endJed;
}

uint32_t dph::EphemerisRelease::index() const
{
    return m_index;
}

const std::string& dph::EphemerisRelease::label() const
{
    return m_label;
}

double dph::EphemerisRelease::constant(const std::string& constantName) const
{
    if (m_ready == false)
    {
        return 0.0;
    }
    else if (constantName == "AU")
    {
        return m_au;
    }
    else if (constantName == "EMRAT")
    {
        return m_emrat;
    }
    else if (constantName == "DENUM")
    {
        return m_index;
    }
    else
    {
        return m_constants.find(constantName)->second;
    }
}

std::string dph::EphemerisRelease::cutBackSpaces(const char* s, size_t size)
{
    for (size_t i = size - 1; i > 0; --i)
    {
        if (s[i] == ' ' && s[i - 1] != ' ')
        {
            return std::string(s, i);
        }
    }

    return std::string(s, size);
}

void dph::EphemerisRelease::clear()
{
    m_ready = false;

    m_filePath.clear();
    m_file.close();

    m_label.clear();
    m_index = 0;
    m_beginJed = 0.0;
    m_endJed = 0.0;
    m_blockTimeSpan = 0.0;
    std::memset(m_keys, 0, sizeof(m_keys));
    m_au = 0.0;
    m_emrat = 0.0;
    std::map<std::string, double>().swap(m_constants); // SWAP TRICK

    m_blocksCount = 0;
    m_ncoeff = 0;
    m_dimensionFit = 0;
    m_blockSize_bytes = 0;

    std::vector<double>().swap(m_buffer); // SWAP TRICK
    std::vector<double>(1).swap(m_poly);  // SWAP TRICK
    std::vector<double>(2).swap(m_dpoly); // SWAP TRICK

    m_poly[0]  = 1;
    m_dpoly[0] = 0;
    m_dpoly[1] = 1;
}

void dph::EphemerisRelease::copyHere(const EphemerisRelease& other)
{
    // Используется в:
    // - Конструктор копирования.
    // - Оператор копирования.

    m_ready = other.m_ready;

    m_filePath = other.m_filePath;

    m_file.close();
    m_file.open(other.m_filePath.c_str(), std::ios::binary);

    m_label = other.m_label;
    m_index = other.m_index;
    m_beginJed = other.m_beginJed;
    m_endJed = other.m_endJed;
    m_blockTimeSpan = other.m_blockTimeSpan;
    std::memcpy(m_keys, other.m_keys, sizeof(m_keys));
    m_au = other.m_au;
    m_emrat = other.m_emrat;
    m_constants = other.m_constants;

    m_blocksCount = other.m_blocksCount;
    m_ncoeff = other.m_ncoeff;
    m_emrat2 = other.m_emrat2;
    m_dimensionFit = other.m_dimensionFit;
    m_blockSize_bytes = other.m_blockSize_bytes;

    m_buffer = other.m_buffer;
    m_poly = other.m_poly;
    m_dpoly = other.m_poly;
}

void dph::EphemerisRelease::readAndPackData()
{
    // Буфферы для чтения информации из файла:

    // Строк. инф. о выпуске.
    char releaseLabel_buffer[RLS_LABELS_COUNT][RLS_LABEL_SIZE]; 

    // Имена констант.
    char constantsNames_buffer[CCOUNT_MAX_NEW][CNAME_SIZE];
    
    // Значения констант.
    double constantsValues_buffer[CCOUNT_MAX_NEW];

    // Количество констант в файле эфемерид:
    uint32_t constantsCount;
    // ---------------------------- Чтение файла ---------------------------- //

    m_file.seekg(0, std::ios::beg);
    m_file.read((char*)&releaseLabel_buffer, RLS_LABEL_SIZE * RLS_LABELS_COUNT);
    m_file.read((char*)&constantsNames_buffer, CNAME_SIZE * CCOUNT_MAX_OLD);
    m_file.read((char*)&m_beginJed, 8);
    m_file.read((char*)&m_endJed, 8);
    m_file.read((char*)&m_blockTimeSpan, 8);
    m_file.read((char*)&constantsCount, 4);
    m_file.read((char*)&m_au, 8);
    m_file.read((char*)&m_emrat, 8);
    m_file.read((char*)&m_keys, (12 * 3) * 4);
    m_file.read((char*)&m_index, 4);
    m_file.read((char*)&m_keys[12], (3) * 4);

    // Чтение дополнительных констант:
    if (constantsCount > 400)
    {
        // Количество дополнительных констант:
        size_t extraConstantsCount = constantsCount - CCOUNT_MAX_OLD;

        m_file.read((char*)&constantsNames_buffer[CCOUNT_MAX_OLD],
            extraConstantsCount * CNAME_SIZE);
    }

    // Чтение дополнительных ключей:
    m_file.read((char*)&m_keys[13], (3 * 2) * 4);

    // Подсчёт ncoeff (количество коэффициентов в блоке):
    m_ncoeff = 2;
    for (int i = 0; i < 15; ++i)
    {
        // Количество компонент для выбранного элемента:
        int comp = i == 11 ? 2 : i == 14 ? 1 : 3;
        m_ncoeff += comp * m_keys[i][1] * m_keys[i][2];
    }

    // Переход к блоку с константами и их чтение:
    if (constantsCount <= CCOUNT_MAX_NEW)
    {
        m_file.seekg(m_ncoeff * 8, std::ios::beg);
        m_file.read((char*)&constantsValues_buffer, constantsCount * 8);
    }


    // -------- Форматирование и упаковка считанной информации -------------- //

    // Формирование строк общей информации о выпуске:
    for (size_t i = 0; i < RLS_LABELS_COUNT; ++i)
    {
        m_label += cutBackSpaces(releaseLabel_buffer[i], RLS_LABEL_SIZE);
        m_label += '\n';
    }

    // Заполнение контейнера m_constants именами и значениями констант:
    if (constantsCount > 0 && constantsCount <= CCOUNT_MAX_NEW)
    {
        for (uint32_t i = 0; i < constantsCount; ++i)
        {
            std::string constantName = cutBackSpaces(constantsNames_buffer[i], CNAME_SIZE);
            m_constants[constantName] = constantsValues_buffer[i];
        }
    }

    // Дополнительные вычисления:
    additionalCalculations();
}

void dph::EphemerisRelease::additionalCalculations()
{
    // Определение доп. коэффициентов для работы с ежегодником:
    m_emrat2 = 1 / (1 + m_emrat);
    m_dimensionFit = 1 / (43200 * m_blockTimeSpan);

    // Определение количества блоков в ежегоднике:
    m_blocksCount = size_t((m_endJed - m_beginJed) / m_blockTimeSpan);

    // Подсчёт максимального количества полиномов в выпуске:
    size_t maxPolynomsCount = 0;
    for (int i = 0; i < 15; ++i)
    {
        if (m_keys[i][1] > maxPolynomsCount)
        {
            maxPolynomsCount = m_keys[i][1];
        }
    }

    // Определение размера блока в байтах:
    m_blockSize_bytes = m_ncoeff * sizeof(double);

    // Резервирование памяти в векторах:
    m_buffer.resize(m_ncoeff);
    m_poly.resize(maxPolynomsCount);
    m_dpoly.resize(maxPolynomsCount);
}

bool dph::EphemerisRelease::isDataCorrect() const
{
    // В данном методе проверяются только те параметры, которые
    // могут повлиять непосредственно на вычисления значений элементов,
    // хранящихся в выпуске эфемерид.

    if (m_file.is_open() == false) return false; // Ошибка открытия файла.
    if (m_beginJed >= m_endJed) return false;
    if (m_blockTimeSpan == 0) return false;
    if ((m_endJed - m_beginJed) < m_blockTimeSpan) return false;
    if (m_emrat == 0) return false;
    if (m_ncoeff == 0) return false;

    if (check_blocksDates() == false) return false;

    return true;
}

bool dph::EphemerisRelease::check_blocksDates() const
{
    // Адрес первого блока с коэффициентами в файле:
    size_t firstBlockAdress = m_blockSize_bytes * 2;

    // Переход к первому блоку:
    m_file.seekg(firstBlockAdress, std::ios::beg);

    // Смещение между блоками после чтения двух первых коэффициентов:
    size_t subBlockOffset = (m_ncoeff - 2) * sizeof(double);

    for (size_t blockIndex = 0; blockIndex < m_blocksCount; ++blockIndex)
    {
        // Массив для чтения первых двух коэффициентов из текущего блока:
        double blockDates[2] = {0.0, 0.0};

        // Чтение:
        m_file.read((char*)& blockDates, sizeof(blockDates));

        // Значения, которые должны быть:
        double blockStartDate = m_beginJed + blockIndex * m_blockTimeSpan;
        double blockEndDate = blockStartDate + m_blockTimeSpan;

        if (blockDates[0] != blockStartDate || blockDates[1] != blockEndDate)
        {
            return false;
        }

        // Переход к следующему блоку:
        m_file.seekg(subBlockOffset, std::ios::cur);
    }

    return true;
}

void dph::EphemerisRelease::fillBuffer(size_t blockNum) const
{
    size_t adress = (2 + blockNum) * m_blockSize_bytes;

    m_file.seekg(adress, std::ios::beg);

    m_file.read((char*)&m_buffer[0], (m_ncoeff) * 8);
}

void dph::EphemerisRelease::interpolatePosition(double normalizedTime,
    unsigned baseItem, const double* coeffs, unsigned componentsCount,
    double* res) const
{
    // Копирование значения количества коэффициентов на компоненту:
    uint32_t cpec = m_keys[baseItem][1];

    // Предварительное заполнение полиномов (вычисление их сумм):
    m_poly[1] = normalizedTime;

    // Заполнение полиномов (вычисление их сумм):
    for (uint32_t i = 2; i < cpec; ++i)
    {
        m_poly[i] = 2 * normalizedTime * m_poly[i - 1] - m_poly[i - 2];
    }

    // Обнуление массива результата вычислений:
    memset(res, 0, sizeof(double) * componentsCount);

    // Вычисление координат:
    for (unsigned i = 0; i < componentsCount; ++i)
    {
        for (uint32_t j = 0; j < cpec; ++j)
        {
            res[i] += m_poly[j] * coeffs[i * cpec + j];
        }
    }
}

void dph::EphemerisRelease::interpolateState(double normalizedTime,
    unsigned baseItem, const double* coeffs, unsigned componentsCount,
    double* res) const
{
    // Копирование значения количества коэффициентов на компоненту:
    uint32_t cpec = m_keys[baseItem][1];

    // Предварительное заполнение полиномов (вычисление их сумм):
    m_poly[1]  = normalizedTime;
    m_poly[2]  = 2 * normalizedTime * normalizedTime - 1;
    m_dpoly[2] = 4 * normalizedTime;

    // Заполнение полиномов (вычисление их сумм):
    for (uint32_t i = 3; i < cpec; ++i)
    {
        m_poly[i] = 2 * normalizedTime *  m_poly[i - 1] -  m_poly[i - 2];
        m_dpoly[i] = 2 * m_poly[i - 1] + 2 * normalizedTime * m_dpoly[i - 1] -
            m_dpoly[i - 2];
    }

    // Обнуление массива результата вычислений:
    memset(res, 0, sizeof(double) * componentsCount * 2);

    // Определение переменной для соблюдения размерности:
    double derivative_units = m_keys[baseItem][2] * m_dimensionFit;

    // Вычисление координат:
    for (unsigned i = 0; i < componentsCount; ++i)
    {
        for (uint32_t j = 0; j < cpec; ++j, ++coeffs)
        {
            res[i]                   +=  m_poly[j] * *coeffs;
            res[i + componentsCount] += m_dpoly[j] * *coeffs;
        }

        res[i + componentsCount] *= derivative_units;
    }
}

void dph::EphemerisRelease::calculateBaseItem(unsigned resType,
    double jed, unsigned baseItem, double* res) const
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

    // Норм. время относительно всех блоков в выпуске:
    double normalizedTime = (jed - m_beginJed) / m_blockTimeSpan;

    // Порядковый номер блока, соотв. заданной дате JED (целая часть от
    // normalizedTime):
    size_t offset = static_cast<size_t>(normalizedTime);

    // Заполнение буффера коэффициентами требуемого блока.
    // Если требуемый блок уже в кэше объекта, то он не заполняется повторно.
    // m_buffer[0] - дата начала блока.
    // m_buffer[1] - дата окончания блока.
    if (jed < m_buffer[0] || jed >= m_buffer[1])
    {
        // Если JED равна последней доступоной дате для вычислений, то
        // заполняется последний блок.

        fillBuffer(offset - (jed == m_endJed ? 1 : 0));
    }

    if (jed == m_endJed)
    {
        // Порядковый номер подблока (последний подблок):
        offset = m_keys[baseItem][2] - 1;

        // Норм. время относительно подблока (в диапазоне от -1 до 1):
        normalizedTime = 1;
    }
    else
    {
        // Норм. время относительно всех подблоков:
        normalizedTime = (normalizedTime - offset) * m_keys[baseItem][2];

        // Порядковый номер подблока (целая часть от normalizedTime):
        offset = static_cast<size_t>(normalizedTime);

        // Норм. время относительно подблока (в диапазоне от -1 до 1):
        normalizedTime = 2 * (normalizedTime - offset) - 1;
    }

    // Количество компонент для выбранного базового элемента:
    unsigned componentsCount = baseItem == 11 ? 2 : baseItem == 14 ? 1 : 3;

    // Порядковый номер первого коэффициента в блоке:
    int coeff_pos  = m_keys[baseItem][0] - 1 + componentsCount * offset *
        m_keys[baseItem][1];

    // Выбор метода вычисления в зависимости от заданного результата вычислений:
    switch(resType) {
    case CALC_POS:
        interpolatePosition(normalizedTime, baseItem, &m_buffer[coeff_pos],
            componentsCount, res);
        break;
    case CALC_STATE:
        interpolateState(normalizedTime, baseItem, &m_buffer[coeff_pos], 
            componentsCount, res);
        break;
    default:
        memset(res, 0, componentsCount * sizeof(double));
    }
}

void dph::EphemerisRelease::calculateBaseEarth(double jed, unsigned resType,
    double* res) const
{
    // Получение радиус-вектора (или вектора состояния) барицентра сиситемы
    // Земля-Луна относительно барицентра Солнечной Системы:
    calculateBaseItem(resType, jed, 2, res);

    // Получение радиус-вектора (или вектора состояния) Луны относитльно Земли:
    double MoonRelativeEarth[6];
    calculateBaseItem(resType, jed, 9, MoonRelativeEarth);

    // Количество компонент:
    unsigned componentsCount = resType == CALC_POS ? 3 : 6;

    // Опредление положения Земли относительно барицентра Солнечной Системы:
    for (unsigned i = 0; i < componentsCount; ++i)
    {
        res[i] -= MoonRelativeEarth[i] * m_emrat2;
    }
}

void dph::EphemerisRelease::calculateBaseMoon(double jed, unsigned resType,
    double* res) const
{
    // Получение радиус-вектора (или вектора состояния) барицентра сиситемы
    // Земля-Луна относительно барицентра Солнечной Системы:
    calculateBaseItem(resType, jed, 2, res);

    // Получение радиус-вектора (или вектора состояния) Луны относитльно Земли:
    double MoonRelativeEarth[6];
    calculateBaseItem(resType, jed, 9, MoonRelativeEarth);

    // Количество компонент:
    unsigned componentsCount = resType == CALC_POS ? 3 : 6;

    // Определение относительного положения:
    for (unsigned i = 0; i < componentsCount; ++i)
    {
        res[i] += MoonRelativeEarth[i] * (1 - m_emrat2);
    }
}
