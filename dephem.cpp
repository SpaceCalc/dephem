#include "dephem.h"

dph::EphemerisRelease::EphemerisRelease(const std::string& binaryFilePath) : 
	m_binaryFilePath(binaryFilePath)
{			
	// Открытие файла:
	m_binaryFileStream = std::fopen(this->m_binaryFilePath.c_str(), "rb");

	// Файл открыт?
	bool isFileOpen = m_binaryFileStream != nullptr;

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
		m_binaryFilePath.clear();
	}
}

dph::EphemerisRelease::EphemerisRelease(const EphemerisRelease& other)
{
	if (other.m_ready)
	{
		copy(other);

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

dph::EphemerisRelease& dph::EphemerisRelease::operator=(const EphemerisRelease& other)
{
	if (other.m_ready)
	{
		clear();
		copy(other);

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

dph::EphemerisRelease::EphemerisRelease(EphemerisRelease&& other) noexcept
{
	if (other.m_ready)
	{
		move(other);

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

dph::EphemerisRelease& dph::EphemerisRelease::operator=(EphemerisRelease&& other) noexcept
{
	if (other.m_ready)
	{
		move(other);

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
	// Закрытие файла эфемерид:
	if (m_binaryFileStream != nullptr)
	{
		std::fclose(m_binaryFileStream);
	}		
}

void dph::EphemerisRelease::calculateBody(unsigned calculationResult,
	unsigned targetBody, unsigned centerBody, double JED, double* resultArray) const
{
	// Допустимые значения параметров:
	// -------------------------------
	//	- calculationResult:
	//		1 - Получить значение радиус-вектора,
	//		2 - Получить значение вектора состояния
	//		Примечание: используй значения из dph::Calculate.
	//
	//	- targetBody, centerBody:
	//		------------------------------------
	//		Индекс	Название
	//		------------------------------------
	//		1		Меркурий 
	//		2		Венера
	//		3		Земля
	//		4		Марс
	//		5		Юпитер
	//		6		Сатурн
	//		7		Уран
	//		8		Нептун
	//		9		Плутон
	//		10		Луна
	//		11		Солнце
	//		12		Барицентр Солнечной Системы
	//		13		Барицентр системы Земля-Луна
	//		------------------------------------
	//		Примечание: используй значения из dph::Body.
	//
	//	- JED:
	//		JED должен принадлежать промежутку: [m_startDate : m_endDate].
	//
	//	- resultArray:
	//		От пользователя требуется знать, каков минимальный размер массива для 
	//		выбранного результата вычислений. Не должен быть нулевым указателем.


	//Условия недопустимые для данного метода:
	if (this->m_ready == false)
	{
		return;
	}
	else if (calculationResult > 1)
	{
		return;
	}
	else if (targetBody == 0 || centerBody == 0)
	{
		return;
	}
	else if (targetBody > 13 || centerBody > 13)
	{
		return;
	}
	else if (JED < m_startDate || JED > m_endDate)
	{
		return;
	}
	else if (resultArray == nullptr)
	{
		return;
	}

	// Количество требуемых компонент:
	unsigned componentsCount = calculationResult == Calculate::STATE ? 6 : 3;

	// Выбор методики вычисления в зависимости от комбинации искомого и центрального тела:
	if (targetBody == centerBody)
	{
		// Случай #1 : Искомое тело является центральным.
		//
		// Результатом является нулевой вектор.

		// Заполнение массива нулями:
		std::memset(resultArray, 0, sizeof(double) * componentsCount);
	}
	else if (targetBody == Body::SSBARY || centerBody == Body::SSBARY)
	{
		// Случай #2: искомым или центральным телом является барицентр Солнечной Системы.
		//
		// Так как все методы calculateBase для тел возвращают вектор относительно барцентра СС,
		// то достаточно вычислить пололжение второго тела. В случае, если искомым телом является
		// сам барицентр СС, то возвращается "зеркальный" вектор второго тела.

		// Индекс тела, что не является барицентром СС:
		unsigned notSSBARY = targetBody == Body::SSBARY ? centerBody : targetBody;

		// Выбор метода вычисления в зависимости от тела:
		switch (notSSBARY)
		{
		case Body::EARTH: calculateBaseEarth(JED, calculationResult, resultArray);	break;
		case Body::MOON: calculateBaseMoon(JED, calculationResult, resultArray);		break;
		case Body::EMBARY: calculateBaseItem(2, JED, calculationResult, resultArray);	break;
		default: calculateBaseItem(notSSBARY - 1, JED, calculationResult, resultArray);
		}

		// Если барицентр СС является искомым телом, то возвращается "зеркальный" вектор:
		if (targetBody == Body::SSBARY)
		{
			for (unsigned i = 0; i < componentsCount; ++i)
			{
				resultArray[i] = -resultArray[i];
			}
		}
	}
	else if (targetBody * centerBody == 30 && targetBody + centerBody == 13)
	{
		// Случай #3 : Искомым и центральным телами являетса Земля и Луна (или Луна и Земля).
		//
		// В этом случае достаточно получить значение положения Луны относительно Земли (базовый 
		// элемент #9 (от нуля). В случае, если искомым телом является Земля, то возвращается
		// "зеркальный вектор".

		// Получение радиус-вектора (или вектора состояния) Луны относительно Земли:
		calculateBaseItem(9, JED, calculationResult, resultArray);

		// Если искомым телом является Земля, то возвращается "зеркальный" вектор.
		if (targetBody == Body::EARTH)
		{
			for (unsigned i = 0; i < componentsCount; ++i)
			{
				resultArray[i] = -resultArray[i];
			}
		}
	}
	else
	{
		// Случай #4 : Все остальные комбинации тел.
		//
		// Сначала вычисляется значение центрального тела относительно барицентра СС, 
		// после - искомого. Результатом является разница между вектором центрального тела и
		// искомого. 

		// Массив для центрального тела:
		double centerBodyArray[6]{};

		// Две итерации:
		for (unsigned i = 0; i <= 1; ++i)
		{
			// Определение индекса и массива в зависимости от номера итерации.
			// i == 0 : работа с центральным телом.
			// i == 1 : работа с искомым телом.
			unsigned currentBodyIndex = i == 0 ? centerBody : targetBody;
			double* currentArray = i == 0 ? centerBodyArray : resultArray;

			// Выбор метода вычисления в зависимости от тела:
			switch (currentBodyIndex)
			{
			case Body::EARTH: calculateBaseEarth(JED, calculationResult, currentArray);	break;
			case Body::MOON: calculateBaseMoon(JED, calculationResult, currentArray);	break;
			case Body::EMBARY: calculateBaseItem(2, JED, calculationResult, currentArray);	break;
			default: calculateBaseItem(currentBodyIndex - 1, JED, calculationResult, currentArray);
			}
		}

		// Разница между вектором центрального и искомого тела:
		for (unsigned i = 0; i < componentsCount; ++i)
		{
			resultArray[i] -= centerBodyArray[i];
		}
	}
}

void dph::EphemerisRelease::calculateOther(unsigned calculationResult,
	unsigned otherItem, double JED,
	double* resultArray) const
{
	// Допустимые значения параметров:
	// -------------------------------
	//	- calculationResult:
	//		1 - Получить оригинальное значение,
	//		2 - Получить оригинальное значение и его (их) производные первого порядка.
	//		Примечание: используй значения из dph::Calculate.
	//	
	//	- other Item:
	//		----------------------------------------------------------------		
	//		Индекс	Название
	//		----------------------------------------------------------------
	//		14		Земные нутации по долдготе и наклонению (модель IAU 1980)	
	//		15		Либрации лунной мантии
	//		16		Угловые скорости лунной мантии
	//		17		TT - TDB (в центре Земли).
	//		----------------------------------------------------------------
	//		Примечание: используй значения из dph::Other.
	//
	//	- JED:
	//		JED должен принадлежать промежутку: [m_startDate : m_endDate].
	//
	//	- resultArray:
	//		От пользователя требуется знать, каков минимальный размер массива для 
	//		выбранного результата вычислений. Не должен быть нулевым указателем.

	//Условия недопустимые для данного метода:
	if (this->m_ready == false)
	{
		return;
	}
	else if (calculationResult > 1)
	{
		return;
	}
	else if (otherItem < 14 || otherItem > 17)
	{
		return;
	}
	else if (JED < m_startDate || JED > m_endDate)
	{
		return;
	}
	else if (resultArray == nullptr)
	{
		return;
	}
	else
	{
		calculateBaseItem(otherItem - 3, JED, calculationResult, resultArray);
	}
}


bool dph::EphemerisRelease::isReady() const
{
	return m_ready;
}

double dph::EphemerisRelease::startDate() const
{
	return m_startDate;
}

double dph::EphemerisRelease::endDate() const
{
	return m_endDate;
}

uint32_t dph::EphemerisRelease::releaseIndex() const
{
	return m_releaseIndex;
}

const std::string& dph::EphemerisRelease::releaseLabel() const
{
	return m_releaseLabel;
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
		return m_releaseIndex;
	}
	else
	{
		return m_constants.at(constantName);
	}
}

std::string dph::EphemerisRelease::cutBackSpaces(const char* charArray, size_t arraySize)
{
	for (size_t i = arraySize - 1; i > 0; --i)
	{
		if (charArray[i] == ' ' && charArray[i - 1] != ' ')
		{
			return std::string(charArray, i);
		}
	}

	return std::string(charArray, arraySize);
}

void dph::EphemerisRelease::clear()
{
	m_ready = false;

	m_binaryFilePath.clear();
	if (m_binaryFileStream != nullptr)
	{
		fclose(m_binaryFileStream);
		m_binaryFileStream = nullptr;
	}		

	m_releaseLabel.clear();
	m_releaseIndex = 0;
	m_startDate = 0.0;
	m_endDate = 0.0;
	m_blockTimeSpan = 0.0;
	std::memset(m_keys, 0, sizeof(m_keys));
	m_au = 0.0;
	m_emrat = 0.0;
	std::map<std::string, double>().swap(m_constants);	// SWAP TRICK

	m_blocksCount = 0;
	m_ncoeff = 0;
	m_maxCheby = 0;
	m_dimensionFit = 0;

	std::vector<double>().swap(m_buffer);	// SWAP TRICK
	std::vector<double>().swap(m_poly);		// SWAP TRICK
	std::vector<double>().swap(m_dpoly);	// SWAP TRICK
}

void dph::EphemerisRelease::copy(const EphemerisRelease& other)
{
	// Используется в:
	//	- Конструктор копирования.
	//	- Оператор копирования.
	
	m_ready = other.m_ready;

	m_binaryFilePath	= other.m_binaryFilePath;
	if (m_binaryFileStream != nullptr)
	{
		std::fclose(m_binaryFileStream);
	}
	m_binaryFileStream = std::fopen(other.m_binaryFilePath.c_str(), "rb");

	m_releaseLabel =	other.m_releaseLabel;
	m_releaseIndex =	other.m_releaseIndex;
	m_startDate =		other.m_startDate;
	m_endDate =			other.m_endDate;
	m_blockTimeSpan =	other.m_blockTimeSpan;
	std::memcpy(m_keys, other.m_keys, sizeof(m_keys));
	m_au =				other.m_au;
	m_emrat =			other.m_emrat;
	m_constants =		other.m_constants;

	m_blocksCount =		other.m_blocksCount;
	m_ncoeff =			other.m_ncoeff;
	m_maxCheby =		other.m_maxCheby;
	m_emrat2 =			other.m_emrat2;
	m_dimensionFit =	other.m_dimensionFit;

	m_buffer =	other.m_buffer;
	m_poly =	other.m_poly;
	m_dpoly =	other.m_poly;
}

void dph::EphemerisRelease::move(EphemerisRelease& other)
{
	// Используется в:
	//	- Конструктор перемещения.
	//	- Оператор перемещения.
	
	m_ready =				other.m_ready;

	m_binaryFilePath =		std::move(other.m_binaryFilePath);

	// Работа с файлом:
	if (m_binaryFileStream != nullptr)
	{
		std::fclose(m_binaryFileStream);
	}
	m_binaryFileStream = other.m_binaryFileStream;
	other.m_binaryFileStream = nullptr;

	m_releaseLabel =		std::move(other.m_releaseLabel);	// Перемещение.
	m_releaseIndex =		other.m_releaseIndex;
	m_startDate =			other.m_startDate;
	m_endDate =				other.m_endDate;
	m_blockTimeSpan =		other.m_blockTimeSpan;
	std::memcpy(m_keys,		other.m_keys, sizeof(m_keys));
	m_au =					other.m_au;
	m_emrat =				other.m_emrat;
	m_constants =			other.m_constants;

	m_blocksCount =			other.m_blocksCount;
	m_ncoeff =				other.m_ncoeff;
	m_maxCheby =			other.m_maxCheby;
	m_emrat2 =				other.m_emrat2;
	m_dimensionFit =		other.m_dimensionFit;

	m_buffer =				std::move(other.m_buffer);			// Перемещение.
	m_poly =				std::move(other.m_poly);			// Перемещение.
	m_dpoly =				std::move(other.m_dpoly);			// Перемещение.
}

void dph::EphemerisRelease::readAndPackData()
{
	// Буфферы для чтения информации из файла:
	char	releaseLabel_buffer[RLS_LABELS_COUNT][RLS_LABEL_SIZE]{};	// Строк. инф. о выпуске.
	char	constantsNames_buffer[CCOUNT_MAX_NEW][CNAME_SIZE]{};		// Имена констант.
	double	constantsValues_buffer[CCOUNT_MAX_NEW]{};					// Значения констант.
	
	// Количество констант в файле эфемерид:
	uint32_t constantsCount{};
	// ------------------------------------- Чтение файла ------------------------------------- //

	std::fread(&releaseLabel_buffer,	RLS_LABEL_SIZE,	RLS_LABELS_COUNT,	m_binaryFileStream);
	std::fread(&constantsNames_buffer,	CNAME_SIZE,		CCOUNT_MAX_OLD,		m_binaryFileStream);
	std::fread(&m_startDate,			8,				1,					m_binaryFileStream);
	std::fread(&m_endDate,				8,				1,					m_binaryFileStream);
	std::fread(&m_blockTimeSpan,		8,				1,					m_binaryFileStream);
	std::fread(&constantsCount,			4,				1,					m_binaryFileStream);
	std::fread(&m_au,					8,				1,					m_binaryFileStream);
	std::fread(&m_emrat,				8,				1,					m_binaryFileStream);
	std::fread(&m_keys,					4,				12 * 3,				m_binaryFileStream);
	std::fread(&m_releaseIndex,			4,				1,					m_binaryFileStream);
	std::fread(&m_keys[12],				4,				3,					m_binaryFileStream);		

	// Чтение дополнительных констант:
	if (constantsCount > 400)
	{
		// Количество дополнительных констант:
		size_t extraConstantsCount = constantsCount - CCOUNT_MAX_OLD;

		std::fread(constantsNames_buffer[400], CNAME_SIZE, extraConstantsCount, 
			m_binaryFileStream);
	}		

	// Чтение дополнительных ключей:
	std::fread(&m_keys[13], sizeof(uint32_t), 3 * 2, m_binaryFileStream);


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
		std::fseek(m_binaryFileStream, m_ncoeff * 8, 0);
		std::fread(constantsValues_buffer, sizeof(double), constantsCount, m_binaryFileStream);
	}
	

	// -------------------- Форматирование и упаковка считанной информации --------------------- // 
	
	// Формирование строк общей информации о выпуске:
	for (size_t i = 0; i < RLS_LABELS_COUNT; ++i)
	{
		m_releaseLabel += cutBackSpaces(releaseLabel_buffer[i], RLS_LABEL_SIZE);
		m_releaseLabel += '\n';
	}
	m_releaseLabel.shrink_to_fit();

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
	m_blocksCount = size_t((m_endDate - m_startDate) / m_blockTimeSpan);

	// Подсчёт max_cheby:
	for (int i = 0; i < 15; ++i)
	{
		if (m_keys[i][1] > m_maxCheby) m_maxCheby = m_keys[i][1];
	}

	// Резервирование памяти в векторах:
	m_buffer.resize(m_ncoeff);
	m_poly.resize(m_maxCheby);
	m_dpoly.resize(m_maxCheby);
}

bool dph::EphemerisRelease::isDataCorrect() const
{
	if (m_binaryFileStream == nullptr)	return false;
	if (m_ncoeff == 0)					return false;
	if (m_startDate >= m_endDate)		return false;
	if (m_blockTimeSpan == 0)			return false;
	if (m_maxCheby == 0)				return false;
	if (m_emrat == 0)					return false;
	if (m_au == 0)						return false;

	double time[2];
	std::fseek(m_binaryFileStream, 16 * m_ncoeff, 0);
	std::fread(time, 2, 8, m_binaryFileStream);

	if (time[0] != m_startDate)				return false;
	if (time[1] != m_startDate + m_blockTimeSpan)	return false;
	
	unsigned int end_pos = (1 + m_blocksCount) * 8 * m_ncoeff;

	if (end_pos > FSEEK_MAX_OFFSET)
	{
		std::fseek(m_binaryFileStream, FSEEK_MAX_OFFSET, 0);
		std::fseek(m_binaryFileStream, end_pos - FSEEK_MAX_OFFSET, 1); 
	}
	else
	{
		std::fseek(m_binaryFileStream, end_pos, 0);
	}
	
	std::fread(time, 2, 8, m_binaryFileStream);
	if (time[0] != m_endDate - m_blockTimeSpan)	return false;
	if (time[1] != m_endDate)				return false;

	return true;
}

void dph::EphemerisRelease::fillBuffer(size_t block_num) const
{
	size_t adress = (2 + block_num) * m_ncoeff * 8;

	if (adress > FSEEK_MAX_OFFSET)
	{
		std::fseek(m_binaryFileStream, FSEEK_MAX_OFFSET, 0);
		std::fseek(m_binaryFileStream, adress - FSEEK_MAX_OFFSET, 1);
	}
	else
	{
		std::fseek(m_binaryFileStream, adress, 0);
	}

	std::fread(m_buffer.data(), sizeof(double), m_ncoeff, m_binaryFileStream);
}

void dph::EphemerisRelease::interpolatePosition(unsigned baseItemIndex, double normalizedTime,
	const double* coeffArray, unsigned componentsCount, double* resultArray) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = m_keys[baseItemIndex][1];
	
	// Предварительное заполнение полиномов (вычисление их сумм):
	m_poly[1] = normalizedTime;

	// Заполнение полиномов (вычисление их сумм):
	for (uint32_t i = 2; i < cpec; ++i)
	{
		m_poly[i] = 2 * normalizedTime * m_poly[i - 1] - m_poly[i - 2];
	}

	// Обнуление массива результата вычислений:
	memset(resultArray, 0, sizeof(double) * componentsCount);

	// Вычисление координат:
	for (unsigned i = 0; i < componentsCount; ++i)
	{
		for (uint32_t j = 0; j < cpec; ++j)
		{
			resultArray[i] += m_poly[j] * coeffArray[i * cpec + j];
		}
	}
}

void dph::EphemerisRelease::interpolateState(unsigned baseItemIndex, double normalizedTime,
	const double* coeffArray, unsigned componentsCount, double* resultArray) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = m_keys[baseItemIndex][1];

	// Предварительное заполнение полиномов (вычисление их сумм):
	m_poly[1]  = normalizedTime;
	m_poly[2]  = 2 * normalizedTime * normalizedTime - 1;
	m_dpoly[2] = 4 * normalizedTime;

	// Заполнение полиномов (вычисление их сумм):
	for (uint32_t i = 3; i < cpec; ++i)
	{
		 m_poly[i] = 2 * normalizedTime *  m_poly[i - 1] -  m_poly[i - 2];
		m_dpoly[i] = 2 * m_poly[i - 1] + 2 * normalizedTime * m_dpoly[i - 1] - m_dpoly[i - 2];
	}

	// Обнуление массива результата вычислений:
	memset(resultArray, 0, sizeof(double) * componentsCount * 2);

	// Определение переменной для соблюдения размерности:
	double derivative_units = m_keys[baseItemIndex][2] * m_dimensionFit;

	// Вычисление координат:
	for (unsigned i = 0; i < componentsCount; ++i)
	{
		for (uint32_t j = 0; j < cpec; ++j, ++coeffArray)
		{
			resultArray[i]                   +=  m_poly[j] * *coeffArray;
			resultArray[i + componentsCount] += m_dpoly[j] * *coeffArray;
		}

		resultArray[i + componentsCount] *= derivative_units;
	}
}

void dph::EphemerisRelease::calculateBaseItem(unsigned baseItemIndex, double JED, 
	unsigned calculationResult, double* resultArray) const
{
	/*
	0	Mercury
	1   Venus
	2   Earth-Moon barycenter
	3   Mars
	4   Jupiter
	5   Saturn
	6   Uranus
	7   Neptune
	8   Pluto
	9   Moon (geocentric)
	10  Sun
	11  Earth Nutations in longitude and obliquity (IAU 1980 model)
	12  Lunar mantle libration
	13  Lunar mantle angular velocity
	14	TT-TDB (at geocenter)
	*/
	
	double norm_time = (JED - m_startDate) / m_blockTimeSpan;	// Норм. время отн. всех блоков.
	size_t offset    = size_t(norm_time);				// Номер треб. блока.

	if (JED < m_buffer[0] || JED >= m_buffer[1])
	{
		fillBuffer(offset - (JED == m_endDate ? 1 : 0));
	}

	norm_time = (norm_time - offset) * m_keys[baseItemIndex][2];	
	offset    = size_t(norm_time);									
	norm_time = 2 * (norm_time - offset) - 1;	
	
	if (JED == m_endDate)
	{
		offset    = m_keys[baseItemIndex][2] - 1;
		norm_time = 1;
	}
	
	// Порядковый номер первого коэффициента для выбранного подпромежутка:
	int comp_count = baseItemIndex == 11 ? 2 : baseItemIndex == 14 ? 1 : 3;
	int coeff_pos  = m_keys[baseItemIndex][0] - 1 + comp_count * offset * m_keys[baseItemIndex][1];

	// В зависимости от того, что требуется вычислить (радиус-вектор
	// или радиус-вектор и вектор скорости) выбирается соответствующий
	// метод:
	
	switch (calculationResult)
	{
	case Calculate::POSITION : 
		interpolatePosition(baseItemIndex, norm_time, &m_buffer[coeff_pos], comp_count, 
			resultArray);
		break;

	case Calculate::STATE :
		interpolateState(baseItemIndex, norm_time, &m_buffer[coeff_pos], comp_count, 
			resultArray);
		break;
		
	default:
		memset(resultArray, 0, comp_count * sizeof(double));
	}		
}

void dph::EphemerisRelease::calculateBaseEarth(double JED, unsigned calculationResult, 
	double* resultArray) const
{
	// Получение радиус-вектора (или вектора состояния) барицентра сиситемы Земля-Луна
	// относительно барицентра Солнечной Системы:
	calculateBaseItem(2, JED, calculationResult, resultArray);

	// Получение радиус-вектора (или вектора состояния) Луны относитльно Земли:
	double MoonRelativeEarth[6];
	calculateBaseItem(9, JED, calculationResult, MoonRelativeEarth);

	// Количество компонент:
	unsigned componentsCount = calculationResult == Calculate::POSITION ? 3 : 6;

	// Опредление положения Земли относительно барицентра Солнечной Системы:
	for (unsigned i = 0; i < componentsCount; ++i)
	{
		resultArray[i] -= MoonRelativeEarth[i] * m_emrat2;
	}
}

void dph::EphemerisRelease::calculateBaseMoon(double JED, unsigned calculationResult,
	double* resultArray) const
{
	// Получение радиус-вектора (или вектора состояния) барицентра сиситемы Земля-Луна
	// относительно барицентра Солнечной Системы:
	calculateBaseItem(2, JED, calculationResult, resultArray);

	// Получение радиус-вектора (или вектора состояния) Луны относитльно Земли:
	double MoonRelativeEarth[6];
	calculateBaseItem(9, JED, calculationResult, MoonRelativeEarth);

	// Количество компонент:
	unsigned componentsCount = calculationResult == Calculate::POSITION ? 3 : 6;

	// Определение относительного положения:
	for (unsigned i = 0; i < componentsCount; ++i)
	{
		resultArray[i] += MoonRelativeEarth[i] * (1 - m_emrat2);
	}	
}