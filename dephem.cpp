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
		m_buffer = new double[Info.m_ncoeff]{};
	}
}

dph::EphemerisRelease::EphemerisRelease(const EphemerisRelease& other)
{
	// Предварительное копирование:	
	this->m_binaryFilePath = other.m_binaryFilePath;
	this->m_ready     = other.m_ready;
	
	// Копирование при готовности:
	if (m_ready)
	{
		// Попытка открыть файл:
		m_binaryFileStream = std::fopen(this->m_binaryFilePath.c_str(), "rb");

		// Завершение копирования при ошибке открытия файла:
		if (m_binaryFileStream == nullptr)
		{
			m_ready = false;
			return;
		}

		// Копирование основных переменных:
		copy(other);
	}
}

dph::EphemerisRelease& dph::EphemerisRelease::operator=(const EphemerisRelease& other)
{	
	// Проверка на равенство самому себе:
	if (&other == this) return *this;

	// Очистка существующей информации:
	if (m_binaryFileStream != nullptr) std::fclose(m_binaryFileStream);
	delete[] m_buffer;
	delete[] Info.m_constantsValues;
	delete[] m_poly;
	delete[] m_dpoly;

	// Предварительное копирование:
	this->m_binaryFilePath = other.m_binaryFilePath;
	this->m_ready     = other.m_ready;

	// Копирование при готовности:
	if (m_ready)
	{
		// Попытка открыть файл:
		m_binaryFileStream = std::fopen(this->m_binaryFilePath.c_str(), "rb");

		// Завершение копирования при ошибке открытия файла:
		if (m_binaryFileStream == nullptr)
		{
			m_ready = false;
			return *this;
		}
		
		// Копирование основных переменных:
		copy(other);
	}
	
	return *this;
}

dph::EphemerisRelease::~EphemerisRelease()
{
	// Закрытие файла ежегодника:
	if (m_binaryFileStream != nullptr) std::fclose(m_binaryFileStream);

	// Освобождение выделенной памяти:
	delete[] m_buffer;
	delete[] Info.m_constantsValues;
	delete[] m_poly;
	delete[] m_dpoly;
}

double dph::EphemerisRelease::get_const(const char* const_name) const
{
	size_t len = strlen(const_name);
	
	if (this->m_ready == false)
	{
		return 0;
	}
	else if (len > 6)
	{
		return 0;
	}

	for (uint32_t i = 0; i < Info.m_constantsCount; i++)
	{
		bool correct = true;

		for (size_t j = 0; j < len; j++) if (const_name[j] != Info.m_constantsNames[i][j])
		{
			correct = false;
			break;
		}

		if (correct) return Info.m_constantsValues[i];
	}

	return 0;
}

void dph::EphemerisRelease::get_coeff(double * coeff, double JED) const
{
	if (this->m_ready == false)
	{
		return;
	}
	else if (JED < Info.m_startDate || JED > Info.m_endDate)
	{
		return;
	}
	else if (coeff == nullptr)
	{
		return;
	}		
	
	size_t block_num = size_t((JED - Info.m_startDate) / Info.m_blockTimeSpan);

	if (JED < m_buffer[0] || JED >= m_buffer[1])
	{
		fill_buffer(block_num - (JED == Info.m_endDate ? 1 : 0));
	}

	for (size_t i = 0; i < Info.m_ncoeff; ++i)
	{
		coeff[i] = m_buffer[i];
	}		
}

void dph::EphemerisRelease::copy(const EphemerisRelease& other)
{
	this->Info = other.Info;

	Info.m_constantsValues = new double[Info.m_constantsCount];
	memcpy_s(Info.m_constantsValues, sizeof(double) * Info.m_constantsCount, other.Info.m_constantsValues, sizeof(double) * other.Info.m_constantsCount);

	m_buffer = new double[Info.m_ncoeff];
	memcpy_s((void*)this->m_buffer, sizeof(double) * Info.m_ncoeff, other.m_buffer, sizeof(double) * Info.m_ncoeff);

	m_poly = new double[Info.m_maxCheby];
	memcpy_s((void*)this->m_poly, sizeof(double) * Info.m_maxCheby, other.m_poly, sizeof(double) * other.Info.m_maxCheby);

	m_dpoly = new double[Info.m_maxCheby];
	memcpy_s((void*)this->m_dpoly, sizeof(double) * Info.m_maxCheby, other.m_dpoly, sizeof(double) * other.Info.m_maxCheby);
}

void dph::EphemerisRelease::move_swap(EphemerisRelease& other)
{
	// Копирование:
	this->m_ready  = other.m_ready;
	this->Info   = other.Info; // (Некоторые элементы структуры копируются по указателю - перемещаются).

	// Перемещение:
	m_binaryFilePath.swap(other.m_binaryFilePath);
	this->m_binaryFileStream    = other.m_binaryFileStream;
	this->m_buffer = other.m_buffer;
	this->m_poly   = other.m_poly;
	this->m_dpoly  = other.m_dpoly;

	// Очистка объекта копирования:
	other.m_ready            = false;
	other.m_binaryFileStream              = nullptr;
	other.Info.m_constantsValues = nullptr;
	other.m_buffer           = nullptr;
	other.m_poly             = nullptr;
	other.m_dpoly            = nullptr;
}

bool dph::EphemerisRelease::read()
{
	for (int i = 0; i < 3; i++)
	{
		std::fread(Info.releaseLabel[i], 1, 84, m_binaryFileStream);
		Info.releaseLabel[i][84] = '\0';
	}

	std::fread(Info.m_constantsNames,   1, 400 * 6, m_binaryFileStream);
	std::fread(&Info.m_startDate,       8,       1, m_binaryFileStream);
	std::fread(&Info.m_endDate,         8,       1, m_binaryFileStream);
	std::fread(&Info.m_blockTimeSpan, 8, 1, m_binaryFileStream);
	std::fread(&Info.m_constantsCount, 4,       1, m_binaryFileStream);
	std::fread(&Info.m_au,          8,       1, m_binaryFileStream);
	std::fread(&Info.m_emrat,       8,       1, m_binaryFileStream);
	std::fread(Info.m_keys,          4,  12 * 3, m_binaryFileStream);
	std::fread(&Info.m_releaseIndex,       4,       1, m_binaryFileStream);
	std::fread(Info.m_keys[12],      4,       3, m_binaryFileStream);		
	
	Info.m_constantsValues = new double[Info.m_constantsCount];

	if (Info.m_constantsCount > 400)
	{
		std::fread(Info.m_constantsNames[400], 6, Info.m_constantsCount - 400, m_binaryFileStream);
	}		

	std::fread(Info.m_keys[13], sizeof(uint32_t), 3, m_binaryFileStream);
	std::fread(Info.m_keys[14], sizeof(uint32_t), 3, m_binaryFileStream);
	
	// Подсчёт ncoeff + max_cheby + items:
	Info.m_ncoeff = 2;
	for (int i = 0; i < 15; ++i)
	{
		int comp = i == 11 ? 2 : i == 14 ? 1 : 3;
		Info.m_ncoeff += comp * Info.m_keys[i][1] * Info.m_keys[i][2];
	}

	std::fseek(m_binaryFileStream, Info.m_ncoeff * 8, 0);
	std::fread(Info.m_constantsValues, 8, Info.m_constantsCount, m_binaryFileStream);

	post_read_calc();

	return authentic();
}

void dph::EphemerisRelease::post_read_calc()
{
	// Определение доп. коэффициентов для работы с ежегодником:
	Info.m_emrat2 = 1 / (1 + Info.m_emrat);
	Info.m_dimensionFit = 1 / (43200 * Info.m_blockTimeSpan);

	// Определение количества блоков в ежегоднике:
	Info.m_blocksCount = size_t((Info.m_endDate - Info.m_startDate) / Info.m_blockTimeSpan);

	// Подсчёт max_cheby и items:
	for (int i = 0; i < 15; ++i)
	{
		if (Info.m_keys[i][1] > Info.m_maxCheby) Info.m_maxCheby = Info.m_keys[i][1];
		if (Info.m_keys[i][1] != 0) Info.items |= 1 << i;
	}
	m_poly  = new double[Info.m_maxCheby] {1};
	m_dpoly = new double[Info.m_maxCheby] {0, 1};
}

bool dph::EphemerisRelease::authentic() const
{
	if (Info.m_ncoeff == 0)					return false;
	if (Info.m_startDate >= Info.m_endDate)				return false;
	if (Info.m_blockTimeSpan == 0)						return false;
	if (Info.m_maxCheby == 0)				return false;
	if (Info.items == 0)					return false;
	if (Info.m_emrat == 0)					return false;
	if (Info.m_au == 0)						return false;

	double time[2];
	std::fseek(m_binaryFileStream, 16 * Info.m_ncoeff, 0);
	std::fread(time, 2, 8, m_binaryFileStream);

	if (time[0] != Info.m_startDate)				return false;
	if (time[1] != Info.m_startDate + Info.m_blockTimeSpan)	return false;
	
	unsigned int end_pos = (1 + Info.m_blocksCount) * 8 * Info.m_ncoeff;

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
	if (time[0] != Info.m_endDate - Info.m_blockTimeSpan)	return false;
	if (time[1] != Info.m_endDate)				return false;

	return true;
}

void dph::EphemerisRelease::fill_buffer(size_t block_num) const
{
	size_t adress = (2 + block_num) * Info.m_ncoeff * 8;

	if (adress > FSEEK_MAX_OFFSET)
	{
		std::fseek(m_binaryFileStream, FSEEK_MAX_OFFSET, 0);
		std::fseek(m_binaryFileStream, adress - FSEEK_MAX_OFFSET, 1);
	}
	else
	{
		std::fseek(m_binaryFileStream, adress, 0);
	}

	std::fread((void*)m_buffer, sizeof(double), Info.m_ncoeff, m_binaryFileStream);
}

void dph::EphemerisRelease::interpolate(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = Info.m_keys[item][1];
	
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
	uint32_t cpec = Info.m_keys[item][1];

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
	double derivative_units = Info.m_keys[item][2] * Info.m_dimensionFit;

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
	
	double norm_time = (JED - Info.m_startDate) / Info.m_blockTimeSpan;	// Норм. время отн. всех блоков.
	size_t offset    = size_t(norm_time);				// Номер треб. блока.

	if (JED < m_buffer[0] || JED >= m_buffer[1])
	{
		fill_buffer(offset - (JED == Info.m_endDate ? 1 : 0));
	}

	norm_time = (norm_time - offset) * Info.m_keys[item][2];	
	offset    = size_t(norm_time);									
	norm_time = 2 * (norm_time - offset) - 1;	
	
	if (JED == Info.m_endDate)
	{
		offset    = Info.m_keys[item][2] - 1;
		norm_time = 1;
	}
	
	// Порядковый номер первого коэффициента для выбранного подпромежутка:
	int comp_count = item == 11 ? 2 : item == 14 ? 1 : 3;
	int coeff_pos  = Info.m_keys[item][0] - 1 + comp_count * offset * Info.m_keys[item][1];

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
		S[i] -= E_M[i] * Info.m_emrat2;
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
		S[i] += E_M[i] * (1 - Info.m_emrat2);
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
	else if (JED < Info.m_startDate || JED > Info.m_endDate)
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
	else if (JED < Info.m_startDate || JED > Info.m_endDate)
	{
		return;
	}
	else if ( (1 << (item - 2) & Info.items) == 0)
	{
		return;
	}
	else
	{
		get_origin_item(item - 2, JED, res, state);
	}
}
