﻿#ifndef DEPHEM_EPHEMERIS_RELEASE_H
#define DEPHEM_EPHEMERIS_RELEASE_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif 

#include <fstream>
#include <cstring>
#include <stdint.h>
#include <string>
#include <map>
#include <vector>

#include "help.hpp" // Body::..., Other::..., Calculate::...

namespace dph
{	
	
// ************************************************************************** //
//                             EphemerisRelease                               //
//                                                                            //
//      Класс для работы с выпусками DE-эфемерид JPL в бинарном формате       //
// -------------------------------------------------------------------------- //
//                                 Описание                                   //
// -------------------------------------------------------------------------- //
// Объект данного класса является представлением требуемого выпуска           //
// DE-эфемерид.                                                               //
//                                                                            //   
// Возможности:                                                               //
//     - Вычисление значений элементов, хранящихся в выпуске эфемерид.        //
//     - Получение общей информации о выпуске эфемерид.                       //
//     - Хранение и доступ к константам, хранящихся в выпуске эфемерид.       //
//                                                                            //
// ************************************************************************** //
class EphemerisRelease
{
public:
		
// ------------------------ Стандартные методы класса ----------------------- //

	// Конструктор по пути к бинарному файлу эфемерид.
	// -----------------------------------------------
	// Чтение файла, проверка полученных значений.
	explicit EphemerisRelease(const std::string& binaryFilePath);

	// Конструктор копирования.
	// ------------------------
	// Проверка полученных значений и доступ к файлу. 
	// При неудачной проверке объект очищается.
	EphemerisRelease(const EphemerisRelease& other);

	// Оператор копирования.
	// ---------------------
	// Проверка полученных значений и доступ к файлу.
	// При неудачной проверке объект очищается.
	EphemerisRelease& operator=(const EphemerisRelease& other);

	// Деструктор.
	// -----------
	// Просто деструктор.
	~EphemerisRelease();

// ---------------------------- Методы вычислений ----------------------------//

	// Получить значение радиус-вектора (или вектора состояния) выбранного
	// тела относительно другого на заданный момент времени.
	// -------------------------------------------------------------------
	// Параметры метода:
	//
	//	- calculationResult	: Индекс результата вычислений. 
	//                        Используй dph::Calculate.
	//
	//	- targetBody		: Порядковый номер искомого тела 
	//						  Используй dph::Body.
	//
	//	- centerBody		: Порядковый номер центрального тела. 
	//						  Используй dph::Body.
	//
	//	- JED				: Момент времени на который требется произвести 
	//						  вычисления в формате Юлианской Эфемеридной Даты
	//						  (Julian Epehemris Date).						  
	//						  Принадлежит промежутку: [startDate : endDate].
	//
	//	- resultArray		: Указатель на массив для результата вычислений. 
	//                        Убедись, что он имеент минимальный допустимый 
	//                        размер для выбранного результата вычислений.
	// -----------------
	// Примечание: если в метод поданы неверные параметры, то он просто 
	// прервётся.
	// -----------------
	void calculateBody(unsigned calculationResult, unsigned targetBody, 
		unsigned centerBody, double JED, double* resultArray) const;

	// Получить значение(-я) прочих элементов, хранящихся в выпуске эфемерид, 
	// на заданный момент времени.
	// -----------------
	// Параметры метода:
	//
	//	- calculationResult	: Индекс результата вычислений
	//						  Используй dph::Calculate.
	//
	//	- otherItem			: Порядковый номер искомого элемента 
	//                        Используй dph::Other.
	//
	//	- JED				: Момент времени на который требется произвести 
	//                        вычисления в формате Юлианской Эфемеридной Даты
	//						  (Julian Epehemris Date).
	//						  Принадлежит промежутку: [startDate : endDate].
	//
	//	- resultArray		: Указатель на массив для результата вычислений. 
	//                        Убедись, что  он имеент минимальный допустимый 
	//						  размер для выбранного результата вычислений.				  
	// -----------------
	// Примечания: 
	//	1. Если в метод поданы неверные параметры, то он просто прервётся.
	//  2. Не всегда в выпуске эфемерид хранится запрашиваемое тело, убедись 
	//	   в его наличии перед запросом.
	// -----------------
	void calculateOther(unsigned calculationResult, unsigned otherItem, 
		double JED, double* resultArray) const;


// --------------------------------- ГЕТТЕРЫ -------------------------------- //

	// Готовность объекта к использованию.
	bool isReady() const;

	// Первая доступная дата для рассчётов.
	double startDate() const;

	// Последняя доступная дата для рассчётов.
	double endDate() const;

	// Номер выпуска.
	uint32_t releaseIndex() const;

	// Строковая информация о выпуске.
	const std::string& releaseLabel() const;

	// Значение константы по её имени.
	double constant(const std::string& constantName) const;

private:
		
// -------------------------- Внутренние значения --------------------------- //

// .......................... Формат DE-эфемерид ............................ //

	// Кол-во строк Общей Информации (ОИ).
	static const size_t RLS_LABELS_COUNT = 3;

	// Кол-во символов в строке ОИ.	
	static const size_t RLS_LABEL_SIZE = 84;	

	// Кол-во символов в имени константы.
	static const size_t CNAME_SIZE = 6;	

	// Кол-во констант (стар. формат).
	static const size_t CCOUNT_MAX_OLD = 400;	

	// Кол-во констант (нов. формат).
	static const size_t CCOUNT_MAX_NEW = 1000;	  

// ............................ Состояние объекта ............................//

	bool m_ready;	// Готовность объекта к работе.									
		
// ........................... Работа с файлом ...............................//

	std::string	 m_binaryFilePath;				// Путь к файлу эфемерид.	
	mutable std::ifstream m_binaryFileStream;	// Поток чтения файла.	

// ..................... Значения, считанные из файла ....................... //

	std::string		m_releaseLabel;		// Строковая информация о выпуске. 
	uint32_t		m_releaseIndex;		// Номерная часть индекса выпуска. 
	double			m_startDate;		// Дата начала выпуска (JED).         
	double			m_endDate;			// Дата окончания выпуска (JED).      
	double			m_blockTimeSpan;	// Временная протяжённость блока.     
	uint32_t		m_keys[15][3];		// Ключи поиска коэффициентов.      	
	double			m_au;				// Астрономическая единица (км).      
	double			m_emrat;			// Отношение массы Земли к массе Луны.  

	std::map<std::string, double> m_constants;	// Константы выпуска.

// ......... Значения, дополнительно определённные внутри объекта ........... //

	size_t		m_blocksCount;		// Количество блоков в файле.                
	uint32_t	m_ncoeff;			// Количество коэффициентов в блоке.              
	double		m_emrat2;			// Отношение массы Луны к массе Земля-Луна.
	double		m_dimensionFit;		// Значение для соблюдения размерности.
	size_t		m_blockSize_bytes;	// Размер блока в байтах.

// .............. Динамические массивы для работы с выпуском ................ //

	mutable std::vector<double> m_buffer;	// Буффер блока с коэффициентами.
	mutable std::vector<double> m_poly;		// Значения полиномов.
	mutable std::vector<double> m_dpoly;	// Значения производных полиномов.


// -------------------- Приватные методы работы объекта --------------------- //

// .......................... Статические методы ............................ //
		
	// Обрезать повторяющиеся пробелы (' ') с конца массива символов "charArray" 
	// размера "arraySize".
	static std::string cutBackSpaces(const char* charArray, size_t arraySize);

// .............. Дополнения к стандартным публичным методам ................ //

	// Приведение объекта к изначальному состоянию.
	void clear();
		
	// Копирование информации из объекта "other" в текущий объект.
	void copyHere(const EphemerisRelease& other);

// ........... Чтение файла и инициализация внутренних значений ............. //

	//  Чтение файла.
	void readAndPackData();

	// Дополнительные вычисления после чтения файла.
	void additionalCalculations();

	// Проверка значений, хранящихся в объекте и проверка файла.
	bool isDataCorrect() const;

	// Проверка начальных и конечных дат всех блоков в файле.
	// Подтверждает целостность файла и доступность всех коэффициентов.
	// Входит в состав проверки isDataCorrect().
	bool check_blocksDates() const;

	// Заполнение буффера "m_buffer" коэффициентами требуемого блока.
	void fillBuffer(size_t block_num) const;

// .............................. Вычисления ................................ //

	// Интерполяция компонент выбранного базового элемента.
	void interpolatePosition(unsigned baseItemIndex, double normalizedTime, 
		const double* coeffArray, unsigned componentsCount, 
			double* resultArray) const;

	// Интерполяция компонент и их производных выбранного базового элемента.
	void interpolateState(unsigned baseItemIndex, double normalizedTime,
		const double* coeffArray, unsigned componentsCount, 
			double* resultArray) const;

	// Получить значения требуемых компонент базового элемента на выбранный 
	// момент времени.
	void calculateBaseItem(unsigned baseItemIndex, double JED, 
		unsigned calculationResult , double* resultArray) const;

	// Получить значение радиус-вектора (или вектора состояния) Земли 
	// относительно барицентра Солнечной Системы.
	void calculateBaseEarth(double JED, unsigned calculationResult, 
		double* resultArray) const;

	// Получить значение радиу-вектора (или вектора состояния) Луны относительно
	// барицентра Солнечной Системы.
	void calculateBaseMoon(double JED, unsigned calculationResult, 
		double* resultArray) const;

}; // class EphemerisRelease

} // namespace dph

dph::EphemerisRelease::EphemerisRelease(const std::string& binaryFilePath)
{			
	// Инициализация внутренних переменных:
	clear();

	// Копирование пути к файлу:
	m_binaryFilePath = binaryFilePath;
	
	// Открытие файла:
	m_binaryFileStream.open(m_binaryFilePath.c_str(), std::ios::binary);

	// Файл открыт?
	bool isFileOpen = m_binaryFileStream.is_open();

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

dph::EphemerisRelease& dph::EphemerisRelease::operator=(const EphemerisRelease& other)
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
	m_binaryFileStream.close();
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
	else if (resultArray == NULL)
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
		double centerBodyArray[6];

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
	else if (resultArray == NULL)
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
		return m_constants.find(constantName)->second;
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
	m_binaryFileStream.close();

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
	m_dimensionFit = 0;
	m_blockSize_bytes = 0;

	std::vector<double>().swap(m_buffer);	// SWAP TRICK
	std::vector<double>(1).swap(m_poly);		// SWAP TRICK
	std::vector<double>(2).swap(m_dpoly);	// SWAP TRICK

	m_poly[0]  = 1;
	m_dpoly[0] = 0;
	m_dpoly[1] = 1;
}

void dph::EphemerisRelease::copyHere(const EphemerisRelease& other)
{
	// Используется в:
	//	- Конструктор копирования.
	//	- Оператор копирования.
	
	m_ready = other.m_ready;

	m_binaryFilePath	= other.m_binaryFilePath;

	m_binaryFileStream.close();
	m_binaryFileStream.open(other.m_binaryFilePath.c_str(), std::ios::binary);

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
	m_emrat2 =			other.m_emrat2;
	m_dimensionFit =	other.m_dimensionFit;
	m_blockSize_bytes = other.m_blockSize_bytes;

	m_buffer =	other.m_buffer;
	m_poly =	other.m_poly;
	m_dpoly =	other.m_poly;
}

void dph::EphemerisRelease::readAndPackData()
{
	// Буфферы для чтения информации из файла:
	char	releaseLabel_buffer[RLS_LABELS_COUNT][RLS_LABEL_SIZE];	// Строк. инф. о выпуске.
	char	constantsNames_buffer[CCOUNT_MAX_NEW][CNAME_SIZE];		// Имена констант.
	double	constantsValues_buffer[CCOUNT_MAX_NEW];					// Значения констант.
	
	// Количество констант в файле эфемерид:
	uint32_t constantsCount;
	// ------------------------------------- Чтение файла ------------------------------------- //

	m_binaryFileStream.seekg(0, std::ios::beg);
	m_binaryFileStream.read((char*)&releaseLabel_buffer, RLS_LABEL_SIZE * RLS_LABELS_COUNT);
	m_binaryFileStream.read((char*)&constantsNames_buffer, CNAME_SIZE * CCOUNT_MAX_OLD);
	m_binaryFileStream.read((char*)&m_startDate, 8);
	m_binaryFileStream.read((char*)&m_endDate, 8);
	m_binaryFileStream.read((char*)&m_blockTimeSpan, 8);
	m_binaryFileStream.read((char*)&constantsCount, 4);
	m_binaryFileStream.read((char*)&m_au, 8);
	m_binaryFileStream.read((char*)&m_emrat, 8);
	m_binaryFileStream.read((char*)&m_keys, (12 * 3) * 4);
	m_binaryFileStream.read((char*)&m_releaseIndex, 4);
	m_binaryFileStream.read((char*)&m_keys[12], (3) * 4);	

	// Чтение дополнительных констант:
	if (constantsCount > 400)
	{
		// Количество дополнительных констант:
		size_t extraConstantsCount = constantsCount - CCOUNT_MAX_OLD;

		m_binaryFileStream.read((char*)&constantsNames_buffer[CCOUNT_MAX_OLD], 
			extraConstantsCount * CNAME_SIZE);
	}		

	// Чтение дополнительных ключей:
	m_binaryFileStream.read((char*)&m_keys[13], (3 * 2) * 4);

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
		m_binaryFileStream.seekg(m_ncoeff * 8, std::ios::beg);
		m_binaryFileStream.read((char*)&constantsValues_buffer, constantsCount * 8);
	}
	

	// -------------------- Форматирование и упаковка считанной информации --------------------- // 
	
	// Формирование строк общей информации о выпуске:
	for (size_t i = 0; i < RLS_LABELS_COUNT; ++i)
	{
		m_releaseLabel += cutBackSpaces(releaseLabel_buffer[i], RLS_LABEL_SIZE);
		m_releaseLabel += '\n';
	}
	m_releaseLabel;

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
	
	if (m_binaryFileStream.is_open() == false)			return false;	// Ошибка открытия файла.
	if (m_startDate >= m_endDate)						return false;
	if (m_blockTimeSpan == 0)							return false;
	if ((m_endDate - m_startDate) < m_blockTimeSpan)	return false;
	if (m_emrat == 0)									return false;
	if (m_ncoeff == 0)									return false;

	if (check_blocksDates() == false)					return false;

	return true;
}

bool dph::EphemerisRelease::check_blocksDates() const
{
	// Адрес первого блока с коэффициентами в файле:
	size_t firstBlockAdress = m_blockSize_bytes * 2;
	
	// Переход к первому блоку:
	m_binaryFileStream.seekg(firstBlockAdress, std::ios::beg);

	// Смещение между блоками после чтения двух первых коэффициентов:
	size_t subBlockOffset = (m_ncoeff - 2) * sizeof(double);

	for (size_t blockIndex = 0; blockIndex < m_blocksCount; ++blockIndex)
	{
		// Массив для чтения первых двух коэффициентов из текущего блока:
		double blockDates[2] = {0.0, 0.0};

		// Чтение:
		m_binaryFileStream.read((char*)& blockDates, sizeof(blockDates));	

		// Значения, которые должны быть:
		double blockStartDate = m_startDate + blockIndex * m_blockTimeSpan;
		double blockEndDate = blockStartDate + m_blockTimeSpan;

		if (blockDates[0] != blockStartDate || blockDates[1] != blockEndDate)
		{
			return false;
		}
		
		// Переход к следующему блоку:
		m_binaryFileStream.seekg(subBlockOffset, std::ios::cur);
	}

	return true;
}

void dph::EphemerisRelease::fillBuffer(size_t block_num) const
{
	size_t adress = (2 + block_num) * m_blockSize_bytes;

	m_binaryFileStream.seekg(adress, std::ios::beg);

	m_binaryFileStream.read((char*)&m_buffer[0], (m_ncoeff) * 8);
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
	// Допустимые значения переданных параметров:
	//	[1]	baseItemIndex - Индекс базового элемента выпуска (от нуля).
	// 
	//						Нумерация базовых элементов выпуска
	//		-------------------------------------------------------------------
	//		Индекс	Наименование
	//		-------------------------------------------------------------------
	//		0		Mercury
	//		1		Venus
	//		2		Earth-Moon barycenter
	//		3		Mars
	//		4		Jupiter
	//		5		Saturn
	//		6		Uranus
	//		7		Neptune
	//		8		Pluto
	//		9		Moon (geocentric)
	//		10		Sun
	//		11		Earth Nutations in longitude and obliquity (IAU 1980 model)
	//		12		Lunar mantle libration
	//		13		Lunar mantle angular velocity
	//		14		TT-TDB (at geocenter)
	//		-------------------------------------------------------------------
	//	[2] JED - момент времени на который требуется получить требуемые значения.
	//	[3] calculationResult - индекс результата вычисления (см. dph::Calculate).
	//	[4] resultArray - указатель на массив для результата вычислений.

	// Внимание! 
	// В ходе выполнения функции смысл переменных "normalizedTime" и "offset" будет меняться.

	// Норм. время относительно всех блоков в выпуске:
	double normalizedTime = (JED - m_startDate) / m_blockTimeSpan;

	// Порядковый номер блока, соотв. заданной дате JED (целая часть от normalizedTime):
	size_t offset = static_cast<size_t>(normalizedTime);

	// Заполнение буффера коэффициентами требуемого блока.
	// Если требуемый блок уже в кэше объекта, то он не заполняется повторно.
	// m_buffer[0] - дата начала блока.
	// m_buffer[1] - дата окончания блока.
	if (JED < m_buffer[0] || JED >= m_buffer[1])
	{
		// Если JED равна последней доступоной дате для вычислений, то заполняется последний блок.

		fillBuffer(offset - (JED == m_endDate ? 1 : 0));
	}		
	
	if (JED == m_endDate)
	{
		// Порядковый номер подблока (последний подблок):
		offset = m_keys[baseItemIndex][2] - 1;

		// Норм. время относительно подблока (в диапазоне от -1 до 1):
		normalizedTime = 1;
	}
	else
	{
		// Норм. время относительно всех подблоков:
		normalizedTime = (normalizedTime - offset) * m_keys[baseItemIndex][2];

		// Порядковый номер подблока (целая часть от normalizedTime):
		offset = static_cast<size_t>(normalizedTime);

		// Норм. время относительно подблока (в диапазоне от -1 до 1):
		normalizedTime = 2 * (normalizedTime - offset) - 1;
	}
	
	// Количество компонент для выбранного базового элемента:
	unsigned componentsCount = baseItemIndex == 11 ? 2 : baseItemIndex == 14 ? 1 : 3;

	// Порядковый номер первого коэффициента в блоке:
	int coeff_pos  = m_keys[baseItemIndex][0] - 1 + componentsCount * offset * m_keys[baseItemIndex][1];

	// Выбор метода вычисления в зависимости от заданного результата вычислений:
	switch(calculationResult)
	{
	case Calculate::POSITION : 
		interpolatePosition(baseItemIndex, normalizedTime, &m_buffer[coeff_pos], componentsCount,
			resultArray);
		break;

	case Calculate::STATE :
		interpolateState(baseItemIndex, normalizedTime, &m_buffer[coeff_pos], componentsCount,
			resultArray);
		break;
		
	default:
		memset(resultArray, 0, componentsCount * sizeof(double));
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

#endif // DEPHEM_EPHEMERIS_RELEASE_HPP