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
	// ������� ��� ��� EphemerisRelease::calculateBody(...).
	class Body
	{
	public:

		static constexpr unsigned MERCURY	= 1;
		static constexpr unsigned VENUS		= 2;
		static constexpr unsigned EARTH		= 3;
		static constexpr unsigned MARS		= 4;
		static constexpr unsigned JUPITER	= 5;
		static constexpr unsigned SATURN	= 6;
		static constexpr unsigned URANUS	= 7;
		static constexpr unsigned NEPTUNE	= 8;
		static constexpr unsigned PLUTO		= 9;
		static constexpr unsigned MOON		= 10;
		static constexpr unsigned SUN		= 11;
		static constexpr unsigned SSBARY	= 12;
		static constexpr unsigned EMBARY	= 13;

	private:
		Body();
	};

	// ������� ��������� ��� EphemerisRelease::calculateOther(...).
	class Other
	{
	public:

		static constexpr unsigned EARTH_NUTATIONS				= 14;
		static constexpr unsigned LUNAR_MANTLE_LIBRATION		= 15;
		static constexpr unsigned LUNAR_MANTLE_ANGULAR_VELOCITY	= 16;
		static constexpr unsigned TTmTDB						= 17;

	private:
		Other();
	};

	// ����� ���������� ���������� � �������:
	// EphemerisRelease::calculateBody(...) 
	// EphemerisRelease::calculateOther(...)
	class Calculate
	{
	public:

		static constexpr unsigned POSITION = 0;
		static constexpr unsigned STATE = 1;

	private:
		Calculate();
	};
	
	class EphemerisRelease
	{
	public:
		
		// -------------------------------- ����� ������ ������ -------------------------------- //

		// ����������� �� ���� � ��������� ����� ��������.
		explicit EphemerisRelease(const std::string& binaryFilePath);

		// ����������� �����������.
		EphemerisRelease(const EphemerisRelease& other);

		// �������� �����������.
		EphemerisRelease& operator=(const EphemerisRelease& other);

		// ����������� �����������.
		EphemerisRelease(EphemerisRelease&& other) noexcept;

		// �������� �����������.
		EphemerisRelease& operator=(EphemerisRelease&& other) noexcept;

		// ����������.
		~EphemerisRelease();

		// ------------------------------- �������� ������ ������ -------------------------------//

		// �������� �������� ������-������� (��� ������� ���������) ���������� ���� ������������ 
		// ������� �� �������� ������ �������.
		void calculateBody(unsigned calculationResult, unsigned targetBody, unsigned centerBody, 
			double JED, double* resultArray) const;
				double* resultArray) const;

		// �������� ��������(-�) ������ ���������, ���������� � ������� ��������.
		void calculateOther(unsigned calculationResult, 
			unsigned otherItemIndex, double JED,
				double* resultArray) const;


		// -------------------------------------- ������� -------------------------------------- //

		// ����� �� ������ � ������.
		bool isReady() const;

		// ������ ��������� ���� ��� ��������.
		double startDate() const;

		// ��������� ��������� ���� ��� ��������.
		double endDate() const;

		// �������� ����� �������.
		uint32_t releaseIndex() const;

		// �������� �������� ��������� �� � �����.
		double constant(const std::string& constantName) const;

	private:
		
		// ------------------------------ ���������� �������� ---------------------------------- //
		
		// ������������ ��������, �������� � ���������� ���� "long". 
		// ��������� ��� �������� � ������� std::fseek (<cstdio>) � �������� ��������� ��������,
		// ��� �������� ����� ����������� ������ ��������.
		static constexpr size_t FSEEK_MAX_OFFSET = std::numeric_limits<long>::max();

		// ������ DE-�������� //
		static constexpr size_t RLS_LABELS_COUNT{ 3 };	// ���-�� ����� ����� ���������� (��).
		static constexpr size_t RLS_LABEL_SIZE{ 84 };	// ���-�� �������� � ������ ��.
		static constexpr size_t CNAME_SIZE{ 6 };		// ���-�� �������� � ����� ���������.
		static constexpr size_t CCOUNT_MAX_OLD{ 400 };	// ���-�� �������� (����. ������).
		static constexpr size_t CCOUNT_MAX_NEW{ 1000 };	// ���-�� �������� (���. ������).  

		// ���������� ������� � ������.
		bool m_ready{ false };
		
		// ������ � ������ //
		std::string	m_binaryFilePath;				// ���� � ��������� ����� ������� ��������.
		FILE*		m_binaryFileStream{ nullptr };	// ����� ������ �����.

		// ��������, ��������� �� ����� //
		std::string		m_releaseLabel;					// ��������� ���������� � �������. 
		uint32_t		m_releaseIndex{};				// �������� ����� ������� �������. 
		double			m_startDate{};					// ���� ������ ������� (JED).         
		double			m_endDate{};					// ���� ��������� ������� (JED).      
		double			m_blockTimeSpan{};				// ��������� ������������ �����.     
		uint32_t		m_keys[15][3]{};				// ����� ������ �������������.      	
		double			m_au{};							// ��������������� ������� (��).      
		double			m_emrat{};						// ��������� ����� ����� � ����� ����.     
		std::map<std::string, double> m_constants;		// ��������� �������.

		// ��������, ������������� ������������ ������ ������� //
		size_t		m_blocksCount{};	// ���������� ������ � �����.                
		uint32_t	m_ncoeff{};			// ���������� ������������� � �����.         
		uint32_t	m_maxCheby{};		// ���������� ���������� ���� ���������.     
		double		m_emrat2{};			// ��������� ����� ���� � ����� ����� � ����.
		double		m_dimensionFit{};	// �������� ��� ���������� �����������.      

		// ������������ ������� ��� ������ � �������� //
		mutable std::vector<double> m_buffer{};			// ������������ �����, �������� �� �����.
		mutable std::vector<double> m_poly{ 1 };		// �������� ���������.
		mutable std::vector<double> m_dpoly{ 0, 1 };	// �������� ����������� ���������.


		// ------------------------- ���������� ������ ������ ������� -------------------------- //

		// �������� ������������� ������� "symbolToCut" � ����� ������� �������� "charArray" 
		// ������� "arraySize".
		static std::string cutBackSymbols(const char* charArray, size_t arraySize,
			char symbolToCut);

		// ���������� ������� � ������������ ���������.
		void clear();
		
		// ����������� ���������� �� ������� "other" � ������� ������.
		void copy(const EphemerisRelease& other);

		// ����������� ���������� �� ������� "other" � ������� ������.
		void move(EphemerisRelease& other);

		//  ������ �����.
		void readAndPackData();

		// �������������� ���������� ����� ������ �����.
		void additionalCalculations();

		// �������� ��������, ���������� � ������� � �������� �����.
		bool isDataCorrect() const;

		// ���������� ������� "m_buffer" �������������� ���������� �����.
		void fillBuffer(size_t block_num) const;

		// ������������ ��������� ���������� �������� ��������.
		void interpolatePosition(unsigned baseItemIndex, double normalizedTime, 
			const double* coeffArray, unsigned componentsCount, double* resultArray) const;

		// ������������ ��������� � �� ����������� ���������� �������� ��������.
		void interpolateState(unsigned baseItemIndex, double normalizedTime,
			const double* coeffArray, unsigned componentsCount, double* resultArray) const;

		// �������� �������� ��������� ��������� �������� �������� �� ��������� ������ �������.
		void calculateBaseItem(unsigned baseItemIndex, double JED,
			unsigned calculationResult , double* resultArray) const;

		// �������� �������� ������-������� (��� ������� ���������) ����� ������������
		// ���������� ��������� �������.
		void calculateBaseEarth(double JED, unsigned calculationResult, double* resultArray) const;

		// �������� �������� �����-������� (��� ������� ���������) ���� ������������
		// ���������� ��������� �������.
		void calculateBaseMoon(double JED, unsigned calculationResult, double* resultArray) const;
	};
}

#endif