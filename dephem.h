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
		friend class more_info;
	public:
		
		// -------------------------------- ����� ������ ������ -------------------------------- //

		// ����������� �� ���� � ��������� ����� ��������.
		explicit EphemerisRelease(const std::string& binaryFilePath);

		// ����������.
		~EphemerisRelease();


		// ------------------------------- �������� ������ ������ -------------------------------//

		// �������� �������� ������-������� (��� ������� ���������) ���������� ���� ������������ 
		// ������� �� �������� ������ �������.
		void get_body(unsigned target, unsigned center, double JED, double* S, bool state) const;

		// �������� ��������(-�) ������ ���������, ���������� � ������� ��������.
		void get_other(unsigned item, double JED, double* res, bool state) const;


		// -------------------------------------- ������� -------------------------------------- //

		// ����� �� ������ � ������.
		bool is_ready() const;

		// ������ ��������� ���� ��� ��������.
		double startDate() const;

		// ��������� ��������� ���� ��� ��������.
		double endDate() const;

		// �������� ����� �������.
		uint32_t releaseIndex() const;

		// �������� ��������� ���������� � �������.
		const std::string& releaseLabel() const;

		// �������� ��� ��������� �� � �������.
		const std::string& constantName(size_t constantIndex) const;

		// �������� �������� ��������� �� � �������.
		double constantValue(size_t constantIndex) const;

		// �������� �������� ��������� �� � �����.
		double constant(const std::string constantName) const;

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
		uint32_t		m_constantsCount{};				// ���������� �������� � �����.       
		std::string*	m_constantsNames{ nullptr };	// ������ � ������� ��������.         
		double*			m_constantsValues{ nullptr };	// ������ �� ���������� ��������.     

		// ��������, ������������� ������������ ������ ������� //
		size_t		m_blocksCount{};	// ���������� ������ � �����.                
		uint32_t	m_ncoeff{};			// ���������� ������������� � �����.         
		uint32_t	m_maxCheby{};		// ���������� ���������� ���� ���������.     
		double		m_emrat2{};			// ��������� ����� ���� � ����� ����� � ����.
		double		m_dimensionFit{};	// �������� ��� ���������� �����������.      

		// ������������ ������� ��� ������ � �������� //
		mutable const double* m_buffer{ nullptr };	// ������������ �����, �������� �� �����.
		double* m_poly{ nullptr };					// �������� ���������.
		double* m_dpoly{ nullptr };					// �������� ����������� ���������.


		// ------------------------- ���������� ������ ������ ������� -------------------------- //

		// �������� ������������� ������� "symbolToCut" � ����� ������� �������� "charArray" 
		// ������� "arraySize".
		static std::string cutBackSymbols(const char* charArray, size_t arraySize,
			char symbolToCut);

		//  ������ �����.
		bool read();

		// �������������� ���������� ����� ������ �����.
		void post_read_calc();

		// �������� ��������, ���������� � ������� � �������� �����.
		bool authentic() const;

		// ���������� ������� "m_buffer" �������������� ���������� �����.
		void fill_buffer(size_t block_num) const;

		// ������������ ��������� ���������� �������� ��������.
		void interpolate(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const;

		// ������������ ��������� � �� ����������� ���������� �������� ��������.
		void interpolate_derivative(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const;

		// �������� �������� ��������� ��������� �������� �������� �� ��������� ������ �������.
		void get_origin_item(unsigned item, double JED, double* S, bool state) const;

		// �������� �������� ������-������� (��� ������� ���������) ����� ������������
		// ���������� ��������� �������.
		void get_origin_earth(double JED, double* S, bool state) const;

		// �������� �������� �����-������� (��� ������� ���������) ���� ������������
		// ���������� ��������� �������.
		void get_origin_moon(double JED, double* S, bool state) const;
	};
}

#endif