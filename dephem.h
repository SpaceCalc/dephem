#pragma once
#ifndef dephemH
#define dephemH

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <exception>
#include <limits>
#include <string>

class dephem
{
	static constexpr size_t MAX_LONG = std::numeric_limits<long>::max();

public:
	static const unsigned int USE_EXCEPTION = 0x1;
	static const unsigned int NO_CHECK      = 0x2;

private:
	std::string file_path;
	
	FILE* eph = nullptr;

	mutable unsigned int option = 0;

	bool ready = false;			

	struct header_info
	{				
		size_t		block_count = 0;
		size_t		ncoeff      = 0;
		uint32_t	const_count = 0;
		int			denum       = 0;
		uint32_t	key[15][3]{};
		double		start       = 0;
		double		end         = 0;
		double		span        = 0;
		double		au          = 0;
		double		emrat       = 0;
		
		char label[3][85]{};

	private:
		friend class dephem;

		char	const_name[1000][6]{};
		double*	const_value = nullptr;

		double   co_em         = 0;
		double   co_span       = 0;
		uint32_t max_cheby     = 0;
		int      items         = 0;
		int	     derived_items = 0;

	} Info;

	mutable const double* buffer = nullptr;

	double* poly  = nullptr;
	double* dpoly = nullptr;

public:
	explicit dephem(const char* file_path, unsigned int option = 0);

	dephem(const dephem& other);

	dephem(dephem&& other) noexcept;
	
	dephem& operator = (const dephem& other);

	dephem& operator = (dephem&& other) noexcept;

	~dephem();
	
	bool is_ready() const { return ready; }

	const header_info* const info = &Info;

	double get_const(const char* const_name) const;

	void available_items(bool* items, bool derived = false) const;

	void get_coeff(double* coeff, double JED) const;

private:

	void throw_error(const char* message) const;
	
	void copy(const dephem& other);

	void move_swap(dephem& other);
	
	bool read();

	void post_read_calc();

	bool authentic() const;	

	void fill_buffer(size_t block_num) const;
	
	void interpolate(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const;

	void interpolate_derivative(const double* set, unsigned item, double norm_time, double* res, unsigned comp_count) const;

	void get_origin_item(unsigned item, double JED, double* S, bool state) const;

	void get_origin_earth(double JED, double* S, bool state) const;
	
	void get_origin_moon (double JED, double* S, bool state) const;

public:

	void get_body(unsigned target, unsigned center, double JED, double* S, bool state) const;

	void get_other(unsigned item, double JED, double* res, bool state) const;
};

#endif