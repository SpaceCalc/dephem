#include "dephem.h"

dph::EphemerisRelease::EphemerisRelease(const std::string& binaryFilePath) : 
	m_binaryFilePath(binaryFilePath)
{			
	// Попытка открыть файл:
	m_binaryFileStream = std::fopen(this->m_binaryFilePath.c_str(), "rb");
	
	if (m_binaryFileStream == nullptr)	// Ошибка открытия файла.
	{
		return;
	}
	else if (read() == false) // Ошибка чтения файла или инициализации переменных.
	{
		return;
	}
	else // Подготовка объекта прошла успешно.
	{
		m_ready  = true;
	}
}

dph::EphemerisRelease::~EphemerisRelease()
{
	// Закрытие файла ежегодника:
	if (m_binaryFileStream != nullptr) std::fclose(m_binaryFileStream);

	// Освобождение выделенной памяти:
	delete[] m_buffer;
	delete[] m_constantsValues;
	delete[] m_poly;
	delete[] m_dpoly;
}

bool dph::EphemerisRelease::is_ready() const
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

double dph::EphemerisRelease::constant(const std::string constantName) const
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
	else if (m_constantsCount == 0)
	{
		return 0.0;
	}
	else if (m_constantsValues == nullptr || m_constantsNames == nullptr)
	{
		return 0.0;
	}
	else
	{
		for (uint32_t i = 0; i < m_constantsCount; ++i)
		{
			if (constantName == m_constantsNames[i])
			{
				return m_constantsValues[i];
			}
		}

		return 0.0;
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

bool dph::EphemerisRelease::read()
{
	// Буфферы для чтения информации из файла:
	char	releaseLabel_buffer[RLS_LABELS_COUNT][RLS_LABEL_SIZE]{};	// Строк. инф. о выпуске.
	char	constantsNames_buffer[CCOUNT_MAX_NEW][CNAME_SIZE]{};		// Имена констант.
	double	constantsValues_buffer[CCOUNT_MAX_NEW]{};					// Значения констант.
	
	// ------------------------------------- Чтение файла ------------------------------------- //

	std::fread(&releaseLabel_buffer,	RLS_LABEL_SIZE,	RLS_LABELS_COUNT,	m_binaryFileStream);
	std::fread(&constantsNames_buffer,	CNAME_SIZE,		CCOUNT_MAX_OLD,		m_binaryFileStream);
	std::fread(&m_startDate,			8,				1,					m_binaryFileStream);
	std::fread(&m_endDate,				8,				1,					m_binaryFileStream);
	std::fread(&m_blockTimeSpan,		8,				1,					m_binaryFileStream);
	std::fread(&m_constantsCount,		4,				1,					m_binaryFileStream);
	std::fread(&m_au,					8,				1,					m_binaryFileStream);
	std::fread(&m_emrat,				8,				1,					m_binaryFileStream);
	std::fread(&m_keys,					4,				12 * 3,				m_binaryFileStream);
	std::fread(&m_releaseIndex,			4,				1,					m_binaryFileStream);
	std::fread(&m_keys[12],				4,				3,					m_binaryFileStream);		

	// Чтение дополнительных констант:
	if (m_constantsCount > 400)
	{
		// Количество дополнительных констант:
		size_t extraConstantsCount = m_constantsCount - CCOUNT_MAX_OLD;

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
	if (m_constantsCount <= CCOUNT_MAX_NEW)
	{
		std::fseek(m_binaryFileStream, m_ncoeff * 8, 0);
		std::fread(constantsValues_buffer, sizeof(double), m_constantsCount, m_binaryFileStream);
	}
	

	// -------------------- Форматирование и упаковка считанной информации --------------------- // 
	
	// Формирование строк общей информации о выпуске:
	for (size_t i = 0; i < RLS_LABELS_COUNT; ++i)
	{
		m_releaseLabel += cutBackSymbols(releaseLabel_buffer[i], RLS_LABEL_SIZE, ' ');
		m_releaseLabel += '\n';
	}
	m_releaseLabel.shrink_to_fit();

	// Формирование массивов с именами констант и их значениями:
	if (m_constantsCount > 0 && m_constantsCount <= CCOUNT_MAX_NEW)
	{
		// Выделение памяти:
		m_constantsNames  = new std::string[m_constantsCount];
		m_constantsValues = new double[m_constantsCount];

		// Форматирование имён констант:
		for (uint32_t i = 0; i < m_constantsCount; ++i)
		{
			m_constantsNames[i] = cutBackSymbols(constantsNames_buffer[i], CNAME_SIZE, ' ');
		}

		// Копирование значений констант:
		std::memcpy(m_constantsValues, constantsValues_buffer, m_constantsCount);
	}

	// Дополнительные вычисления:
	post_read_calc();

	// Вернуть результат проверки считанной информации:
	return authentic();
}

void dph::EphemerisRelease::post_read_calc()
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
	m_poly  = new double[m_maxCheby] {1};
	m_dpoly = new double[m_maxCheby] {0, 1};

	// Выделение памяти под буффер:
	m_buffer = new double[m_ncoeff] {};
}

bool dph::EphemerisRelease::authentic() const
{
	if (m_ncoeff == 0)					return false;
	if (m_startDate >= m_endDate)				return false;
	if (m_blockTimeSpan == 0)						return false;
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

void dph::EphemerisRelease::fill_buffer(size_t block_num) const
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

	std::fread((void*)m_buffer, sizeof(double), m_ncoeff, m_binaryFileStream);
}

void dph::EphemerisRelease::interpolate(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = m_keys[item][1];
	
	// Предварительное заполнение полиномов (вычисление их сумм):
	m_poly[1] = norm_time;

	// Заполнение полиномов (вычисление их сумм):
	for (uint32_t i = 2; i < cpec; ++i)
	{
		m_poly[i] = 2 * norm_time * m_poly[i - 1] - m_poly[i - 2];
	}

	// Обнуление массива результата вычислений:
	memset(res, 0, sizeof(double) * comp_count);

	// Вычисление координат:
	for (unsigned i = 0; i < comp_count; ++i)
	{
		for (uint32_t j = 0; j < cpec; ++j)
		{
			res[i] += m_poly[j] * set[i * cpec + j];
		}
	}
}

void dph::EphemerisRelease::interpolate_derivative(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = m_keys[item][1];

	// Предварительное заполнение полиномов (вычисление их сумм):
	m_poly[1]  = norm_time;
	m_poly[2]  = 2 * norm_time * norm_time - 1;
	m_dpoly[2] = 4 * norm_time;

	// Заполнение полиномов (вычисление их сумм):
	for (uint32_t i = 3; i < cpec; ++i)
	{
		 m_poly[i] =                   2 * norm_time *  m_poly[i - 1] -  m_poly[i - 2];
		m_dpoly[i] = 2 * m_poly[i - 1] + 2 * norm_time * m_dpoly[i - 1] - m_dpoly[i - 2];
	}

	// Обнуление массива результата вычислений:
	memset(res, 0, sizeof(double) * comp_count * 2);

	// Определение переменной для соблюдения размерности:
	double derivative_units = m_keys[item][2] * m_dimensionFit;

	// Вычисление координат:
	for (unsigned i = 0; i < comp_count; ++i)
	{
		for (uint32_t j = 0; j < cpec; ++j, ++set)
		{
			res[i]              +=  m_poly[j] * *set;
			res[i + comp_count] += m_dpoly[j] * *set;
		}

		res[i + comp_count] *= derivative_units;
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
		fill_buffer(offset - (JED == m_endDate ? 1 : 0));
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
	if (state)	interpolate_derivative(&m_buffer[coeff_pos], item, norm_time, S, comp_count);
	else	    interpolate           (&m_buffer[coeff_pos], item, norm_time, S, comp_count);
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

void dph::EphemerisRelease::get_body(unsigned target, unsigned center, double JED, double* S, bool state) const
{
	/*
	0   Mercury
	1   Venus
	2   Earth
	3   Mars
	4   Jupiter
	5   Saturn
	6   Uranus
	7   Neptune
	8   Pluto
	9   Moon
	10  Sun
	11  Solar System barycenter
	12  Earth-Moon barycenter
	 */

	// Уменьшение индекса на единицу:
	--target;
	--center;

	//Условия недопустимые для данного метода:
	if (this->m_ready == false)
	{
		return;
	}
	else if (target > 12 || center > 12)
	{
		return;
	}
	else if (JED < m_startDate || JED > m_endDate)
	{
		return;
	}

	// Определить количество требуемых компонент:
	unsigned comp_count = state ? 6 : 3;

	if (target == 11 || center == 11)
	{
		unsigned g = target == 11 ? center : target;
		
		if      (g == 12)	get_origin_item (2, JED, S, state);
		else if (g ==  2)	get_origin_earth(   JED, S, state);
		else if (g ==  9)	get_origin_moon (   JED, S, state);
		else				get_origin_item (g, JED, S, state);

		if(target == 11)	for (unsigned i = 0; i < comp_count; ++i)	S[i] = -S[i];
	}
	else if (target * center == 18 && target + center == 11)
	{
		get_origin_item(9, JED, S, state);
		
		if(target == 2)	for (unsigned i = 0; i < comp_count; ++i)	S[i] = -S[i];
	}
	else
	{
		double C[6];

		for (unsigned i = 0; i < 2; ++i)
		{
			double* G = i ? C : S;
			int		g = i ? center : target;

			if      (g == 12)	get_origin_item (2, JED, G, state);
			else if (g ==  2)	get_origin_earth(   JED, G, state);
			else if (g ==  9)	get_origin_moon (   JED, G, state);
			else				get_origin_item (g, JED, G, state);
		}

		for (unsigned i = 0; i < comp_count; ++i)
		{
			S[i] -= C[i];
		}	
	}
}

void dph::EphemerisRelease::get_other(unsigned item, double JED, double* res, bool state) const
{
	/*
	13	Earth Nutations in longitudeand obliquity(IAU 1980 model)
	14	Lunar mantle libration
	15	Lunar mantle angular velocity
	16	TT - TDB(at geocenter)
	*/

	// Уменьшение индекса на единицу:
	--item;

	//Условия недопустимые для данного метода:
	if (this->m_ready == false)
	{
		return;
	}
	else if (item < 13 || item > 16)
	{
		return;
	}
	else if (JED < m_startDate || JED > m_endDate)
	{
		return;
	}
	else
	{
		get_origin_item(item - 2, JED, res, state);
	}
}
