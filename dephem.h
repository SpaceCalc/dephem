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

namespace dph
{
	class EphemerisRelease
	{
		struct header_info;
	public:
		
		// -------------------------------- Общие методы класса -------------------------------- //

		// Конструктор по пути к бинарному файлу эфемерид.
		explicit EphemerisRelease(const std::string& binaryFilePath);

		// Конструктор копирования.
		EphemerisRelease(const EphemerisRelease& other);

		// Оператор копирования.
		EphemerisRelease& operator = (const EphemerisRelease& other);

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
		bool is_ready() const { return m_ready; }

		// Указатель на структуру с параметрами объекта (выпуска).
		const header_info* const info = &Info;

		// Получить значение хранимой константы по её имени.
		double get_const(const char* const_name) const;

		// Заполнить массив коэффициентами блока, соответствующего моменту времени JED.
		void get_coeff(double* coeff, double JED) const;

	private:
		
		// ------------------------------ Внутренние значения ---------------------------------- //
		
		// Максимальное значение, хранимое в переменной типа "long". 
		// Требуется для передачи в функцию std::fseek (<cstdio>) в качестве параметра смещения,
		// при размерах файла превышающих данное значение.
		static constexpr size_t FSEEK_MAX_OFFSET = std::numeric_limits<long>::max();

		// Готовность объекта к работе.
		bool m_ready = false;
		
		// Работа с файлом //
		std::string	m_binaryFilePath;				// Путь к бинарному файлу выпуска эфемерид.
		FILE*		m_binaryFileStream = nullptr;	// Поток чтения файла.

		// Структура с параметрами выпуска и прочими //
		struct header_info
		{
			size_t		m_blocksCount = 0;
			size_t		m_ncoeff = 0;
			uint32_t	m_constantsCount = 0;
			int			m_releaseIndex = 0;
			uint32_t	m_keys[15][3]{};
			double		m_startDate = 0;
			double		m_endDate = 0;
			double		m_blockTimeSpan = 0;
			double		m_au = 0;
			double		m_emrat = 0;

			char releaseLabel[3][85]{};

		private:
			friend class EphemerisRelease;

			char	m_constantsNames[1000][6]{};
			double* m_constantsValues = nullptr;

			double   m_emrat2 = 0;
			double   m_dimensionFit = 0;
			uint32_t m_maxCheby = 0;

		} Info;

		// Динамическик массивы для работы с выпуском //
		mutable const double* m_buffer = nullptr;	// Коэффициенты блока, читаемые из файла.
		double* m_poly = nullptr;					// Значения полиномов.
		double* m_dpoly = nullptr;					// Значения производных полиномов.


		// ------------------------- Внутренние методы работы объекта -------------------------- //

		// Копирования объекта по ссылке.
		void copy(const EphemerisRelease& other);

		// Перемещение объекта по ссылке.
		void move_swap(EphemerisRelease& other);

		//  Чтение файла.
		bool read();

		// Дополнительные вычисления после чтения файла.
		void post_read_calc();

		// Проверка значений, хранящихся в объекте и проверка файла.
		bool authentic() const;

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