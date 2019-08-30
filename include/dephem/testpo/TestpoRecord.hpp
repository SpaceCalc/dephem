#ifndef DEPHEM_TESTPO_TESTPO_RECORD_HPP
#define DEPHEM_TESTPO_TESTPO_RECORD_HPP

#include <string>
#include <cstdlib> // atoi, atof
#include <sstream>
#include <iomanip> // setw, setprecision

namespace dph { namespace testpo {

class Record
{
public:
	// Конструктор по строке записи.
	explicit Record(const std::string& str);

	// Дата вычисления (JED).
	double date() const {return m_date;}
				
	// Значение компоненты.
	double value() const {return m_value;}

	// Порядковый номер искомого тела.
	unsigned target() const {return m_target;}

	// Порядковый номер центрального тела.
	unsigned center() const {return m_center;}

	// Порядковый номер компоненты.
	unsigned component() const {return m_component;}

	// Запись в строковом виде.
	std::string str() const;

private:
	double		m_date;			// Дата вычисления (JED).
	double		m_value;		// Значение компоненты.
	unsigned	m_target;		// Порядковый номер искомого тела.
	unsigned	m_center;		// Порядковый номер центрального тела.
	unsigned	m_component;	// Порядковый номер компоненты.

}; // clas Record

} } // namespace dph::testpo

dph::testpo::Record::Record(const std::string& str)
{
    m_date =		std::atof(str.substr(15, 10).c_str());
	m_target =		std::atoi(str.substr(25, 3).c_str());
	m_center =		std::atoi(str.substr(28, 3).c_str());
	m_component =	std::atoi(str.substr(31, 3).c_str());
	m_value =		std::atof(str.substr(34).c_str());
}

std::string dph::testpo::Record::str() const
{
    std::ostringstream stringStream;

	stringStream.setf(std::ios::fixed);

	stringStream << std::setw(11) << std::setprecision(1) << m_date;
	stringStream << std::setw(3) << m_target;
	stringStream << std::setw(3) << m_center;
	stringStream << std::setw(3) << m_component;
	stringStream << std::setw(30) << std::setprecision(20) << m_value;

	return stringStream.str();
}

#endif // DEPHEM_TESTPO_TESTPO_RECORD_HPP