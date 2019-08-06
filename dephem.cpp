#include "dephem.h"

dph::EphemerisRelease::EphemerisRelease(const char * binaryFilePath) : m_binaryFilePath(binaryFilePath)
{			
	// Попытка открыть файл:
	eph = std::fopen(this->m_binaryFilePath.c_str(), "rb");
	
	if (eph == nullptr)	// Ошибка открытия файла.
	{
		return;
	}
	else if (read() == false) // Ошибка чтения файла или инициализации переменных.
	{
		return;
	}
	else // Подготовка объекта прошла успешно.
	{
		ready  = true;
		buffer = new double[Info.ncoeff]{};
	}
}

dph::EphemerisRelease::EphemerisRelease(const EphemerisRelease& other)
{
	// Предварительное копирование:	
	this->m_binaryFilePath = other.m_binaryFilePath;
	this->ready     = other.ready;
	
	// Копирование при готовности:
	if (ready)
	{
		// Попытка открыть файл:
		eph = std::fopen(this->m_binaryFilePath.c_str(), "rb");

		// Завершение копирования при ошибке открытия файла:
		if (eph == nullptr)
		{
			ready = false;
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
	if (eph != nullptr) std::fclose(eph);
	delete[] buffer;
	delete[] Info.const_value;
	delete[] poly;
	delete[] dpoly;

	// Предварительное копирование:
	this->m_binaryFilePath = other.m_binaryFilePath;
	this->ready     = other.ready;

	// Копирование при готовности:
	if (ready)
	{
		// Попытка открыть файл:
		eph = std::fopen(this->m_binaryFilePath.c_str(), "rb");

		// Завершение копирования при ошибке открытия файла:
		if (eph == nullptr)
		{
			ready = false;
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
	if (eph != nullptr) std::fclose(eph);

	// Освобождение выделенной памяти:
	delete[] buffer;
	delete[] Info.const_value;
	delete[] poly;
	delete[] dpoly;
}

double dph::EphemerisRelease::get_const(const char* const_name) const
{
	size_t len = strlen(const_name);
	
	if (this->ready == false)
	{
		return 0;
	}
	else if (len > 6)
	{
		return 0;
	}

	for (uint32_t i = 0; i < Info.const_count; i++)
	{
		bool correct = true;

		for (size_t j = 0; j < len; j++) if (const_name[j] != Info.const_name[i][j])
		{
			correct = false;
			break;
		}

		if (correct) return Info.const_value[i];
	}

	return 0;
}

void dph::EphemerisRelease::available_items(bool* items, bool derived) const
{
	if (this->ready == false)
	{
		return;
	}
			
	memset(items, 0, (derived ? 17 : 15) * sizeof(bool));

	for (int i = 0; i < (derived ? 17 : 15); i++)
	{
		items[i] = (derived ? Info.derived_items : Info.items) & (1 << i);
	}
}

void dph::EphemerisRelease::get_coeff(double * coeff, double JED) const
{
	if (this->ready == false)
	{
		return;
	}
	else if (JED < Info.start || JED > Info.end)
	{
		return;
	}
	else if (coeff == nullptr)
	{
		return;
	}		
	
	size_t block_num = size_t((JED - Info.start) / Info.span);

	if (JED < buffer[0] || JED >= buffer[1])
	{
		fill_buffer(block_num - (JED == Info.end ? 1 : 0));
	}

	for (size_t i = 0; i < Info.ncoeff; ++i)
	{
		coeff[i] = buffer[i];
	}		
}

void dph::EphemerisRelease::copy(const EphemerisRelease& other)
{
	this->Info = other.Info;

	Info.const_value = new double[Info.const_count];
	memcpy_s(Info.const_value, sizeof(double) * Info.const_count, other.Info.const_value, sizeof(double) * other.Info.const_count);

	buffer = new double[Info.ncoeff];
	memcpy_s((void*)this->buffer, sizeof(double) * Info.ncoeff, other.buffer, sizeof(double) * Info.ncoeff);

	poly = new double[Info.max_cheby];
	memcpy_s((void*)this->poly, sizeof(double) * Info.max_cheby, other.poly, sizeof(double) * other.Info.max_cheby);

	dpoly = new double[Info.max_cheby];
	memcpy_s((void*)this->dpoly, sizeof(double) * Info.max_cheby, other.dpoly, sizeof(double) * other.Info.max_cheby);
}

void dph::EphemerisRelease::move_swap(EphemerisRelease& other)
{
	// Копирование:
	this->ready  = other.ready;
	this->Info   = other.Info; // (Некоторые элементы структуры копируются по указателю - перемещаются).

	// Перемещение:
	m_binaryFilePath.swap(other.m_binaryFilePath);
	this->eph    = other.eph;
	this->buffer = other.buffer;
	this->poly   = other.poly;
	this->dpoly  = other.dpoly;

	// Очистка объекта копирования:
	other.ready            = false;
	other.eph              = nullptr;
	other.Info.const_value = nullptr;
	other.buffer           = nullptr;
	other.poly             = nullptr;
	other.dpoly            = nullptr;
}

bool dph::EphemerisRelease::read()
{
	for (int i = 0; i < 3; i++)
	{
		std::fread(Info.label[i], 1, 84, eph);
		Info.label[i][84] = '\0';
	}

	std::fread(Info.const_name,   1, 400 * 6, eph);
	std::fread(&Info.start,       8,       1, eph);
	std::fread(&Info.end,         8,       1, eph);
	std::fread(&Info.span, 8, 1, eph);
	std::fread(&Info.const_count, 4,       1, eph);
	std::fread(&Info.au,          8,       1, eph);
	std::fread(&Info.emrat,       8,       1, eph);
	std::fread(Info.key,          4,  12 * 3, eph);
	std::fread(&Info.denum,       4,       1, eph);
	std::fread(Info.key[12],      4,       3, eph);		
	
	Info.const_value = new double[Info.const_count];

	if (Info.const_count > 400)
	{
		std::fread(Info.const_name[400], 6, Info.const_count - 400, eph);
	}		

	std::fread(Info.key[13], sizeof(uint32_t), 3, eph);
	std::fread(Info.key[14], sizeof(uint32_t), 3, eph);
	
	// Подсчёт ncoeff + max_cheby + items:
	Info.ncoeff = 2;
	for (int i = 0; i < 15; ++i)
	{
		int comp = i == 11 ? 2 : i == 14 ? 1 : 3;
		Info.ncoeff += comp * Info.key[i][1] * Info.key[i][2];
	}

	std::fseek(eph, Info.ncoeff * 8, 0);
	std::fread(Info.const_value, 8, Info.const_count, eph);

	post_read_calc();

	return authentic();
}

void dph::EphemerisRelease::post_read_calc()
{
	// Определение доп. коэффициентов для работы с ежегодником:
	Info.co_em = 1 / (1 + Info.emrat);
	Info.co_span = 1 / (43200 * Info.span);

	// Определение количества блоков в ежегоднике:
	Info.block_count = size_t((Info.end - Info.start) / Info.span);

	// Подсчёт max_cheby и items:
	for (int i = 0; i < 15; ++i)
	{
		if (Info.key[i][1] > Info.max_cheby) Info.max_cheby = Info.key[i][1];
		if (Info.key[i][1] != 0) Info.items |= 1 << i;
	}
	poly  = new double[Info.max_cheby] {1};
	dpoly = new double[Info.max_cheby] {0, 1};

	// Определение списка производных элементов:
	for (int i = 0; i < 17; ++i)
	{		
		if (Info.items & (i == 2 ? 0x204 : i == 11 ? 0x5FF : i == 12 ? 0x4 : i > 12 ? (1 << (i - 2)) : 1 << i))
		{
			Info.derived_items |= 1 << i;
		}
	}
}

bool dph::EphemerisRelease::authentic() const
{
	if (Info.ncoeff == 0)					return false;
	if (Info.start >= Info.end)				return false;
	if (Info.span == 0)						return false;
	if (Info.max_cheby == 0)				return false;
	if (Info.items == 0)					return false;
	if (Info.emrat == 0)					return false;
	if (Info.au == 0)						return false;

	double time[2];
	std::fseek(eph, 16 * Info.ncoeff, 0);
	std::fread(time, 2, 8, eph);

	if (time[0] != Info.start)				return false;
	if (time[1] != Info.start + Info.span)	return false;
	
	unsigned int end_pos = (1 + Info.block_count) * 8 * Info.ncoeff;

	if (end_pos > MAX_LONG)
	{
		std::fseek(eph, MAX_LONG, 0);
		std::fseek(eph, end_pos - MAX_LONG, 1); 
	}
	else
	{
		std::fseek(eph, end_pos, 0);
	}
	
	std::fread(time, 2, 8, eph);
	if (time[0] != Info.end - Info.span)	return false;
	if (time[1] != Info.end)				return false;

	return true;
}

void dph::EphemerisRelease::fill_buffer(size_t block_num) const
{
	size_t adress = (2 + block_num) * Info.ncoeff * 8;

	if (adress > MAX_LONG)
	{
		std::fseek(eph, MAX_LONG, 0);
		std::fseek(eph, adress - MAX_LONG, 1);
	}
	else
	{
		std::fseek(eph, adress, 0);
	}

	std::fread((void*)buffer, sizeof(double), Info.ncoeff, eph);
}

void dph::EphemerisRelease::interpolate(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = Info.key[item][1];
	
	// Предварительное заполнение полиномов (вычисление их сумм):
	poly[1] = norm_time;

	// Заполнение полиномов (вычисление их сумм):
	for (uint32_t i = 2; i < cpec; ++i)
	{
		poly[i] = 2 * norm_time * poly[i - 1] - poly[i - 2];
	}

	// Обнуление массива результата вычислений:
	memset(res, 0, sizeof(double) * comp_count);

	// Вычисление координат:
	for (unsigned i = 0; i < comp_count; ++i)
	{
		for (uint32_t j = 0; j < cpec; ++j)
		{
			res[i] += poly[j] * set[i * cpec + j];
		}
	}
}

void dph::EphemerisRelease::interpolate_derivative(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const
{
	// Копирование значения количества коэффициентов на компоненту:
	uint32_t cpec = Info.key[item][1];

	// Предварительное заполнение полиномов (вычисление их сумм):
	poly[1]  = norm_time;
	poly[2]  = 2 * norm_time * norm_time - 1;
	dpoly[2] = 4 * norm_time;

	// Заполнение полиномов (вычисление их сумм):
	for (uint32_t i = 3; i < cpec; ++i)
	{
		 poly[i] =                   2 * norm_time *  poly[i - 1] -  poly[i - 2];
		dpoly[i] = 2 * poly[i - 1] + 2 * norm_time * dpoly[i - 1] - dpoly[i - 2];
	}

	// Обнуление массива результата вычислений:
	memset(res, 0, sizeof(double) * comp_count * 2);

	// Определение переменной для соблюдения размерности:
	double derivative_units = Info.key[item][2] * Info.co_span;

	// Вычисление координат:
	for (unsigned i = 0; i < comp_count; ++i)
	{
		for (uint32_t j = 0; j < cpec; ++j, ++set)
		{
			res[i]              +=  poly[j] * *set;
			res[i + comp_count] += dpoly[j] * *set;
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
	
	double norm_time = (JED - Info.start) / Info.span;	// Норм. время отн. всех блоков.
	size_t offset    = size_t(norm_time);				// Номер треб. блока.

	if (JED < buffer[0] || JED >= buffer[1])
	{
		fill_buffer(offset - (JED == Info.end ? 1 : 0));
	}

	norm_time = (norm_time - offset) * Info.key[item][2];	
	offset    = size_t(norm_time);									
	norm_time = 2 * (norm_time - offset) - 1;	
	
	if (JED == Info.end)
	{
		offset    = Info.key[item][2] - 1;
		norm_time = 1;
	}
	
	// Порядковый номер первого коэффициента для выбранного подпромежутка:
	int comp_count = item == 11 ? 2 : item == 14 ? 1 : 3;
	int coeff_pos  = Info.key[item][0] - 1 + comp_count * offset * Info.key[item][1];

	// В зависимости от того, что требуется вычислить (радиус-вектор
	// или радиус-вектор и вектор скорости) выбирается соответствующий
	// метод:
	if (state)	interpolate_derivative(&buffer[coeff_pos], item, norm_time, S, comp_count);
	else	    interpolate           (&buffer[coeff_pos], item, norm_time, S, comp_count);
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
		S[i] -= E_M[i] * Info.co_em;
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
		S[i] += E_M[i] * (1 - Info.co_em);
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
	if (this->ready == false)
	{
		return;
	}
	else if (target > 12 || center > 12)
	{
		return;
	}
	else if (JED < Info.start || JED > Info.end)
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
	if (this->ready == false)
	{
		return;
	}
	else if (item < 13 || item > 16)
	{
		return;
	}
	else if (JED < Info.start || JED > Info.end)
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
