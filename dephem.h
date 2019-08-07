#pragma once
#ifndef dephemH
#define dephemH

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif 

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <limits>
#include <string>
#include <map>
#include <vector>

namespace dph
{
	class EphemerisRelease
	{
		friend class more_info;
	public:
		
		// -------------------------------- Общие методы класса -------------------------------- //

		// Конструктор по пути к бинарному файлу эфемерид.
		explicit EphemerisRelease(const std::string& binaryFilePath);

		// Конструктор копирования.
		EphemerisRelease(const EphemerisRelease& other);

		// Деструктор.
		~EphemerisRelease();

		// ------------------------------- Основные методы класса -------------------------------//

		// Получить значение радиус-вектора (или вектора состояния) выбранного тела относительно 
		// другого на заданный момент времени.
		void get_body(unsigned target, unsigned center, double JED, double* S, bool state) const;

		// Получить значение(-я) прочих элементов, хранящихся в выпуске эфемерид.
		void get_other(unsigned item, double JED, double* res, bool state) const;


		// -------------------------------------- ГЕТТЕРЫ -------------------------------------- //

		// Готов ли объект к работе.
		bool is_ready() const;

		// Первая доступная дата для рассчёта.
		double startDate() const;

		// Последняя доступная дата для рассчёта.
		double endDate() const;

		// Получить номер выпуска.
		uint32_t releaseIndex() const;

		// Получить значение константы по её имени.
		double constant(const std::string& constantName) const;

	private:
		
		// ------------------------------ Внутренние значения ---------------------------------- //
		
		// Максимальное значение, хранимое в переменной типа "long". 
		// Требуется для передачи в функцию std::fseek (<cstdio>) в качестве параметра смещения,
		// при размерах файла превышающих данное значение.
		static constexpr size_t FSEEK_MAX_OFFSET = std::numeric_limits<long>::max();

		// Формат DE-эфемерид //
		static constexpr size_t RLS_LABELS_COUNT{ 3 };	// Кол-во строк Общей Информации (ОИ).
		static constexpr size_t RLS_LABEL_SIZE{ 84 };	// Кол-во символов в строке ОИ.
		static constexpr size_t CNAME_SIZE{ 6 };		// Кол-во символов в имени константы.
		static constexpr size_t CCOUNT_MAX_OLD{ 400 };	// Кол-во констант (стар. формат).
		static constexpr size_t CCOUNT_MAX_NEW{ 1000 };	// Кол-во констант (нов. формат).  

		// Готовность объекта к работе.
		bool m_ready{ false };
		
		// Работа с файлом //
		std::string	m_binaryFilePath;				// Путь к бинарному файлу выпуска эфемерид.
		FILE*		m_binaryFileStream{ nullptr };	// Поток чтения файла.

		// Значения, считанные из файла //
		std::string		m_releaseLabel;					// Строковая информация о выпуске. 
		uint32_t		m_releaseIndex{};				// Номерная часть индекса выпуска. 
		double			m_startDate{};					// Дата начала выпуска (JED).         
		double			m_endDate{};					// Дата окончания выпуска (JED).      
		double			m_blockTimeSpan{};				// Временная протяжённость блока.     
		uint32_t		m_keys[15][3]{};				// Ключи поиска коэффициентов.      	
		double			m_au{};							// Астрономическая единица (км).      
		double			m_emrat{};						// Отношение массы Земли к массе Луны.     
		std::map<std::string, double> m_constants;		// Константы выпуска.

		// Значения, дополнительно определённные внутри объекта //
		size_t		m_blocksCount{};	// Количество блоков в файле.                
		uint32_t	m_ncoeff{};			// Количество коэффициентов в блоке.         
		uint32_t	m_maxCheby{};		// Наибольшее количество сумм полиномов.     
		double		m_emrat2{};			// Отношение массы Луны к массе Земли и Луны.
		double		m_dimensionFit{};	// Значение для соблюдения размерности.      

		// Динамические массивы для работы с выпуском //
		mutable std::vector<double> m_buffer{};			// Коэффициенты блока, читаемые из файла.
		mutable std::vector<double> m_poly{ 1 };		// Значения полиномов.
		mutable std::vector<double> m_dpoly{ 0, 1 };	// Значения производных полиномов.


		// ------------------------- Внутренние методы работы объекта -------------------------- //

		// Обрезать повторяющиеся символы "symbolToCut" с конца массива символов "charArray" 
		// размера "arraySize".
		static std::string cutBackSymbols(const char* charArray, size_t arraySize,
			char symbolToCut);

		// Приведение объекта к изначальному состоянию.
		void clear();
		
		// Копирование информации из объекта "other" в текущий объект.
		void copy(const EphemerisRelease& other);

		//  Чтение файла.
		void readAndPackData();

		// Дополнительные вычисления после чтения файла.
		void additionalCalculations();

		// Проверка значений, хранящихся в объекте и проверка файла.
		bool isDataCorrect() const;

		// Заполнение буффера "m_buffer" коэффициентами требуемого блока.
		void fill_buffer(size_t block_num) const;

		// Интерполяция компонент выбранного базового элемента.
		void interpolate(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const;

		// Интерполяция компонент и их производных выбранного базового элемента.
		void interpolate_derivative(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const;

		// Получить значения требуемых компонент базового элемента на выбранный момент времени.
		void get_origin_item(unsigned item, double JED, double* S, bool state) const;

		// Получить значение радиус-вектора (или вектора состояния) Земли относительно
		// барицентра Солнечной Системы.
		void get_origin_earth(double JED, double* S, bool state) const;

		// Получить значение радиу-вектора (или вектора состояния) Луны относительно
		// барицентра Солнечной Системы.
		void get_origin_moon(double JED, double* S, bool state) const;
	};
}

#endif