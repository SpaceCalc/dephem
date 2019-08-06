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
		
		// -------------------------------- ����� ������ ������ -------------------------------- //

		// ����������� �� ���� � ��������� ����� ��������.
		explicit EphemerisRelease(const std::string& binaryFilePath);

		// ����������� �����������.
		EphemerisRelease(const EphemerisRelease& other);

		// �������� �����������.
		EphemerisRelease& operator = (const EphemerisRelease& other);

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
		bool is_ready() const { return m_ready; }

		// ��������� �� ��������� � ����������� ������� (�������).
		const header_info* const info = &Info;

		// �������� �������� �������� ��������� �� � �����.
		double get_const(const char* const_name) const;

		// ��������� ������ �������������� �����, ���������������� ������� ������� JED.
		void get_coeff(double* coeff, double JED) const;

	private:
		
		// ------------------------------ ���������� �������� ---------------------------------- //
		
		// ������������ ��������, �������� � ���������� ���� "long". 
		// ��������� ��� �������� � ������� std::fseek (<cstdio>) � �������� ��������� ��������,
		// ��� �������� ����� ����������� ������ ��������.
		static constexpr size_t FSEEK_MAX_OFFSET = std::numeric_limits<long>::max();

		// ���������� ������� � ������.
		bool m_ready = false;
		
		// ������ � ������ //
		std::string	m_binaryFilePath;				// ���� � ��������� ����� ������� ��������.
		FILE*		m_binaryFileStream = nullptr;	// ����� ������ �����.

		// ��������� � ����������� ������� � ������� //
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

		// ������������ ������� ��� ������ � �������� //
		mutable const double* m_buffer = nullptr;	// ������������ �����, �������� �� �����.
		double* m_poly = nullptr;					// �������� ���������.
		double* m_dpoly = nullptr;					// �������� ����������� ���������.


		// ------------------------- ���������� ������ ������ ������� -------------------------- //

		// ����������� ������� �� ������.
		void copy(const EphemerisRelease& other);

		// ����������� ������� �� ������.
		void move_swap(EphemerisRelease& other);

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