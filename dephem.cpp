#include "dephem.h"

dph::EphemerisRelease::EphemerisRelease(const std::string& binaryFilePath) : 
	m_binaryFilePath(binaryFilePath)
{			
	// Открытие файла:
	m_binaryFileStream = std::fopen(this->m_binaryFilePath.c_str(), "rb");

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
	// Закрытие файла ежегодника:
	if (m_binaryFileStream != nullptr) 
		std::fclose(m_binaryFileStream);
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
	if (m_ready == false)
	{
		return 0;
	}
	else
	{
		return m_releaseIndex;
	}
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

std::string dph::EphemerisRelease::cutBackSymbols(const char* charArray, size_t arraySize, char symbolToCut)
{
	for (size_t i = arraySize - 1; i > 0; --i)
	{
		if (charArray[i] == symbolToCut && charArray[i - 1] != symbolToCut)
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
		m_releaseLabel += cutBackSymbols(releaseLabel_buffer[i], RLS_LABEL_SIZE, ' ');
		m_releaseLabel += '\n';
	}
	m_releaseLabel.shrink_to_fit();

	// Заполнение контейнера m_constants именами и значениями констант:
	if (constantsCount > 0 && constantsCount <= CCOUNT_MAX_NEW)
	{
		for (uint32_t i = 0; i < constantsCount; ++i)
		{
			std::string constantName = cutBackSymbols(constantsNames_buffer[i], CNAME_SIZE, ' ');
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

void dph::EphemerisRelease::get_origin_item(unsigned item, double JED, double *S, bool state) const
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

	norm_time = (norm_time - offset) * m_keys[item][2];	
	offset    = size_t(norm_time);									
	norm_time = 2 * (norm_time - offset) - 1;	
	
	if (JED == m_endDate)
	{
		offset    = m_keys[item][2] - 1;
		norm_time = 1;
	}
	
	// Порядковый номер первого коэффициента для выбранного подпромежутка:
	int comp_count = item == 11 ? 2 : item == 14 ? 1 : 3;
	int coeff_pos  = m_keys[item][0] - 1 + comp_count * offset * m_keys[item][1];

	// В зависимости от того, что требуется вычислить (радиус-вектор
	// или радиус-вектор и вектор скорости) выбирается соответствующий
	// метод:
	if (state)	interpolateState	(item, norm_time, &m_buffer[coeff_pos], comp_count, S);
	else	    interpolatePosition	(item, norm_time, &m_buffer[coeff_pos], comp_count, S);
}

void dph::EphemerisRelease::get_origin_earth(double JED, double* S, bool state) const
{
	// Получение ВС барицентра З-Л:
	get_origin_item(2, JED, S, state);

	// Получение ВС Луны (относительно Земли):
	double E_M[6];
	get_origin_item(9, JED, E_M, state);

	// Определение относительного положения:
	for (int i = 0; i < int(state ? 6 : 3); ++i)
	{
		S[i] -= E_M[i] * m_emrat2;
	}
}

void dph::EphemerisRelease::get_origin_moon(double JED, double* S, bool state) const
{
	// Получение ВС барицентра З-Л:
	get_origin_item(2, JED, S, state);

	// Получение ВС Луны (относительно Земли):
	double E_M[6];
	get_origin_item(9, JED, E_M, state);

	// Определение относительного положения:
	for (int i = 0; i < int(state ? 6 : 3); ++i)
	{
		S[i] += E_M[i] * (1 - m_emrat2);
	}	
}

void dph::EphemerisRelease::calculateBody(unsigned targetBodyIndex, unsigned centerBodyIndex, double JED,
	bool calculateState, double* resultArray) const
{
	/*
	1   Mercury
	2   Venus
	3   Earth
	4   Mars
	5   Jupiter
	6   Saturn
	7   Uranus
	8   Neptune
	9   Pluto
	10  Moon
	11  Sun
	12  Solar System barycenter
	13  Earth-Moon barycenter
	 */

	//Условия недопустимые для данного метода:
	if (this->m_ready == false)
	{
		return;
	}
	else if (targetBodyIndex == 0 || centerBodyIndex == 0)
	{
		return;
	}
	else if (targetBodyIndex > 13 || centerBodyIndex > 13)
	{
		return;
	}
	else if (JED < m_startDate || JED > m_endDate)
	{
		return;
	}

	// Определить количество требуемых компонент:
	unsigned componentsCount = calculateState ? 6 : 3;

	if (targetBodyIndex == 12 || centerBodyIndex == 12)
	{
		unsigned notSSBARY = targetBodyIndex == 12 ? centerBodyIndex : targetBodyIndex;
		
		if      (notSSBARY == 13)	get_origin_item(2, JED, resultArray, calculateState);
		else if (notSSBARY ==  3)	get_origin_earth(JED, resultArray, calculateState);
		else if (notSSBARY == 10)	get_origin_moon(JED, resultArray, calculateState);
		else						
			get_origin_item(notSSBARY - 1, JED, resultArray, calculateState);

		if (targetBodyIndex == 12)
		{
			for (unsigned i = 0; i < componentsCount; ++i)
			{
				resultArray[i] = -resultArray[i];
			}
		}				
	}
	else if (targetBodyIndex * centerBodyIndex == 30 && targetBodyIndex + centerBodyIndex == 13)
	{
		get_origin_item(9, JED, resultArray, calculateState);
		
		if (targetBodyIndex == 3)
		{
			for (unsigned i = 0; i < componentsCount; ++i)
			{
				resultArray[i] = -resultArray[i];
			}
		}				
	}
	else
	{
		double centerBodyArray[6]{};

		for (unsigned i = 0; i <= 1; ++i)
		{
			double*  currentArray =		i == 0 ? centerBodyArray : resultArray;
			unsigned currentBodyIndex =	i == 0 ? centerBodyIndex : targetBodyIndex;

			switch (currentBodyIndex)
			{
			case 3:		get_origin_earth(JED, currentArray, calculateState);	break;
			case 10:	get_origin_moon(JED, currentArray, calculateState);		break;
			case 13:	get_origin_item(2, JED, currentArray, calculateState);	break;
			default:	get_origin_item(currentBodyIndex - 1, JED, currentArray, calculateState);
			}
		}

		for (unsigned i = 0; i < componentsCount; ++i)
		{
			resultArray[i] -= centerBodyArray[i];
		}	
	}
}

void dph::EphemerisRelease::calculateOther(unsigned otherItemIndex, double JED,
	bool calculateDerivative, double* resultArray) const
{
	/*
	14	Earth Nutations in longitudeand obliquity(IAU 1980 model)
	15	Lunar mantle libration
	16	Lunar mantle angular velocity
	17	TT - TDB(at geocenter)
	*/

	//Условия недопустимые для данного метода:
	if (this->m_ready == false)
	{
		return;
	}
	else if (otherItemIndex < 14 || otherItemIndex > 17)
	{
		return;
	}
	else if (JED < m_startDate || JED > m_endDate)
	{
		return;
	}
	else
	{
		get_origin_item(otherItemIndex - 3, JED, resultArray, calculateDerivative);
	}
}
