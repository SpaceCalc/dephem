﻿#pragma once
#ifndef EPHEMERIS_RELEASE_H
#define EPHEMERIS_RELEASE_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif 

#include <fstream>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <limits>
#include <string>
#include <map>
#include <vector>

namespace dph
{
	// ***************************************************************************************** //
	//                                               Body                                        //
	//                                                                                           //
	//                                           Индексы тел                                     //
	// ----------------------------------------------------------------------------------------- //
	//                                             Описание                                      //
	// ----------------------------------------------------------------------------------------- //
	// Вспомогательный класс, хранящий значения параметров для метода                            //
	// dph::EphemerisReelase::calculateBody(...).                                                //
	//                                                                                           //
	// Значения хранимых констант соответствуют индексам тел, для которых можно получить         //
	// результат вычислений.                                                                     //
	//                                                                                           //
	// ***************************************************************************************** //
	class Body
	{
	public:

		static const unsigned MERCURY	= 1;
		static const unsigned VENUS		= 2;
		static const unsigned EARTH		= 3;
		static const unsigned MARS		= 4;
		static const unsigned JUPITER	= 5;
		static const unsigned SATURN	= 6;
		static const unsigned URANUS	= 7;
		static const unsigned NEPTUNE	= 8;
		static const unsigned PLUTO		= 9;
		static const unsigned MOON		= 10;
		static const unsigned SUN		= 11;
		static const unsigned SSBARY	= 12;	// Барицентр Солнечной Системы.
		static const unsigned EMBARY	= 13;	// Барицентр системы Земля-Луна.

	private:
		Body(); // Запрет на создание объекта типа Body.
	};

	// ***************************************************************************************** //
	//                                              Other                                        //
	//                                                                                           //
	//                                    Индексы прочих элементов                               //
	// ----------------------------------------------------------------------------------------- //
	//                                             Описание                                      //
	// ----------------------------------------------------------------------------------------- //
	// Вспомогательный класс, хранящий значения параметров для метода                            //
	// dph::EphemerisReelase::calculateOther(...).                                               //
	//                                                                                           //
	// Значения хранимых констант соответствуют индексам прочих элементов, для которых можно     //
	// получить результат вычислений.                                                            //
	//                                                                                           //
	// ***************************************************************************************** //
	class Other
	{
	public:

		static const unsigned EARTH_NUTATIONS				= 14;
		static const unsigned LUNAR_MANTLE_LIBRATION		= 15;
		static const unsigned LUNAR_MANTLE_ANGULAR_VELOCITY	= 16;
		static const unsigned TTmTDB						= 17;

	private:
		Other(); // Запрет на создание объекта типа Other.
	};

	// ***************************************************************************************** //
	//                                              Other                                        //
	//                                                                                           //
	//                                    Индексы результатов вычислений                         //
	// ----------------------------------------------------------------------------------------- //
	//                                             Описание                                      //
	// ----------------------------------------------------------------------------------------- //
	// Вспомогательный класс, хранящий значения параметров для методов:                          //
	//    dph::EphemerisReelase::calculateBody(...),                                             //
	//    dph::EphemerisReelase::calculateOther(...).                                            //
	//                                                                                           //
	// Значения хранимых констант соответствуют индексам результатов вычислений, которые можно   // 
	// получить.                                                                                 //
	//                                                                                           //
	// ***************************************************************************************** //
	class Calculate
	{
	public:

		static const unsigned POSITION	= 0;
		static const unsigned STATE		= 1;

	private:
		Calculate(); // Запрет на создание объекта типа Calculate.
	};
	
	// ***************************************************************************************** //
	//                                          EphemerisRelease                                 //
	//                                                                                           //
	//                   Класс для работы с выпусками DE-эфемерид JPL в бинарном формате         //
	// ----------------------------------------------------------------------------------------- //
	//                                             Описание                                      //
	// ----------------------------------------------------------------------------------------- //
	// Объект данного класса является представлением требуемого выпуска DE-эфемерид.             //
	//                                                                                           //   
	// Возможности:																				 //
	//     - Вычисление значений элементов, хранящихся в выпуске эфемерид.						 //
	//     - Получение общей информации о выпуске эфемерид.										 //
	//     - Хранение и доступ к константам, хранящихся в выпуске эфемерид.						 //
	// 																							 //
	// ***************************************************************************************** //
	class EphemerisRelease
	{
	public:
		
		// ----------------------------- Стандартные методы класса ----------------------------- //

		// Конструктор по пути к бинарному файлу эфемерид.
		// Чтение файла, проверка полученных значений.
		explicit EphemerisRelease(const std::string& binaryFilePath);

		// Конструктор копирования.
		// Проверка полученных значений и доступ к файлу. При неудачной проверке объект очищается.
		EphemerisRelease(const EphemerisRelease& other);

		// Оператор копирования.
		// Проверка полученных значений и доступ к файлу. При неудачной проверке объект очищается.
		EphemerisRelease& operator=(const EphemerisRelease& other);

		// Деструктор.
		// Просто деструктор.
		~EphemerisRelease();

		// --------------------------------- Методы вычислений ----------------------------------//

		// Получить значение радиус-вектора (или вектора состояния) выбранного тела относительно 
		// другого на заданный момент времени.
		// -----------------
		// Параметры метода:
		//
		//	- calculationResult	: Индекс результата вычислений (используй dph::Calculate).
		//	- targetBody		: Порядковый номер искомого тела (используй dph::Body).
		//	- centerBody		: Порядковый номер центрального тела. (используй dph::Body).
		//	- JED				: Момент времени на который требется произвести вычисления в
		//						  формате Юлианской Эфемеридной Даты (Julian Epehemris Date).
		//						  Принадлежит промежутку: [.startDate() : .endDate()].
		//	- resultArray		: Указатель на массив для результата вычислений. Убедись, что
		//						  он имеент минимальный допустимый размер для выбранного
		//						  результата вычислений.
		// -----------------
		// Примечание: если в метод поданы неверные параметры, то он просто прервётся.
		// -----------------
		void calculateBody(unsigned calculationResult, unsigned targetBody, unsigned centerBody, 
			double JED, double* resultArray) const;

		// Получить значение(-я) прочих элементов, хранящихся в выпуске эфемерид, на заданный
		// момент времени.
		// -----------------
		// Параметры метода:
		//
		//	- calculationResult	: Индекс результата вычислений (используй dph::Calculate).
		//	- otherItem			: Порядковый номер искомого элемента (используй dph::Other).
		//	- JED				: Момент времени на который требется произвести вычисления в
		//						  формате Юлианской Эфемеридной Даты (Julian Epehemris Date).
		//						  Принадлежит промежутку: [.startDate() : .endDate()].
		//	- resultArray		: Указатель на массив для результата вычислений. Убедись, что
		//						  он имеент минимальный допустимый размер для выбранного
		//						  результата вычислений.
		// -----------------
		// Примечания: 
		//	1. Если в метод поданы неверные параметры, то он просто прервётся.
		//  2. Не всегда в выпуске эфемерид хранится запрашиваемое тело, убедись в его наличии
		//	   перед запросом.
		// -----------------
		void calculateOther(unsigned calculationResult, unsigned otherItem, 
			double JED, double* resultArray) const;


		// -------------------------------------- ГЕТТЕРЫ -------------------------------------- //

		// Готов ли объект к работе.
		bool isReady() const;

		// Первая доступная дата для рассчёта.
		double startDate() const;

		// Последняя доступная дата для рассчёта.
		double endDate() const;

		// Получить номер выпуска.
		uint32_t releaseIndex() const;

		// Получить строковую информацию о выпуске.
		const std::string& releaseLabel() const;

		// Получить значение константы по её имени.
		double constant(const std::string& constantName) const;

	private:
		
		// ------------------------------ Внутренние значения ---------------------------------- //

		// Максимальное значение, хранимое в переменной типа "long". 
		// Требуется для передачи в функцию std::fseek (<cstdio>) в качестве параметра смещения,
		// при размерах файла превышающих данное значение.
		static const size_t FSEEK_MAX_OFFSET;

		// ................................. Формат DE-эфемерид ................................ //

		static const size_t RLS_LABELS_COUNT	= 3;	// Кол-во строк Общей Информации (ОИ).
		static const size_t RLS_LABEL_SIZE		= 84;	// Кол-во символов в строке ОИ.
		static const size_t CNAME_SIZE			= 6;	// Кол-во символов в имени константы.
		static const size_t CCOUNT_MAX_OLD		= 400;	// Кол-во констант (стар. формат).
		static const size_t CCOUNT_MAX_NEW		= 1000;	// Кол-во констант (нов. формат).  

		// ................................. Состояние объекта ..................................//

		bool m_ready;									// Готовность объекта к работе.
		
		// .................................. Работа с файлом ...................................//

		std::string				m_binaryFilePath;		// Путь к бинарному файлу выпуска эфемерид.
		mutable std::ifstream	m_binaryFileStream;	// Поток чтения файла.

		// ............................ Значения, считанные из файла ........................... //

		std::string		m_releaseLabel;					// Строковая информация о выпуске. 
		uint32_t		m_releaseIndex;					// Номерная часть индекса выпуска. 
		double			m_startDate;					// Дата начала выпуска (JED).         
		double			m_endDate;						// Дата окончания выпуска (JED).      
		double			m_blockTimeSpan;				// Временная протяжённость блока.     
		uint32_t		m_keys[15][3];					// Ключи поиска коэффициентов.      	
		double			m_au;							// Астрономическая единица (км).      
		double			m_emrat;						// Отношение массы Земли к массе Луны.     
		std::map<std::string, double> m_constants;		// Константы выпуска.

		// ............... Значения, дополнительно определённные внутри объекта ................ //

		size_t		m_blocksCount;						// Количество блоков в файле.                
		uint32_t	m_ncoeff;							// Количество коэффициентов в блоке.              
		double		m_emrat2;							// Отношение массы Луны к массе Земля-Луна.
		double		m_dimensionFit;						// Значение для соблюдения размерности.
		size_t		m_blockSize_bytes;					// Размер блока в байтах.

		// .................... Динамические массивы для работы с выпуском ..................... //

		mutable std::vector<double> m_buffer;			// Коэффициенты блока, читаемые из файла.
		mutable std::vector<double> m_poly;				// Значения полиномов.
		mutable std::vector<double> m_dpoly;			// Значения производных полиномов.


		// -------------------------- Приватные методы работы объекта -------------------------- //

		// ................................. Статические методы ................................ //
		
		// Обрезать повторяющиеся пробелы (' ') с конца массива символов "charArray" 
		// размера "arraySize".
		static std::string cutBackSpaces(const char* charArray, size_t arraySize);

		// ..................... Дополнения к стандартным публичным методам .................... //

		// Приведение объекта к изначальному состоянию.
		void clear();
		
		// Копирование информации из объекта "other" в текущий объект.
		void copyHere(const EphemerisRelease& other);

		// ................. Чтение файла и инициализация внутренних значений .................. //

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

		// .................................... Вычисления ..................................... //

		// Интерполяция компонент выбранного базового элемента.
		void interpolatePosition(unsigned baseItemIndex, double normalizedTime, 
			const double* coeffArray, unsigned componentsCount, double* resultArray) const;

		// Интерполяция компонент и их производных выбранного базового элемента.
		void interpolateState(unsigned baseItemIndex, double normalizedTime,
			const double* coeffArray, unsigned componentsCount, double* resultArray) const;

		// Получить значения требуемых компонент базового элемента на выбранный момент времени.
		void calculateBaseItem(unsigned baseItemIndex, double JED,
			unsigned calculationResult , double* resultArray) const;

		// Получить значение радиус-вектора (или вектора состояния) Земли относительно
		// барицентра Солнечной Системы.
		void calculateBaseEarth(double JED, unsigned calculationResult, double* resultArray) const;

		// Получить значение радиу-вектора (или вектора состояния) Луны относительно
		// барицентра Солнечной Системы.
		void calculateBaseMoon(double JED, unsigned calculationResult, double* resultArray) const;
	};
}

#endif