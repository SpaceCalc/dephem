#ifndef DEPHEM_TESTPO_TESTPO_FILTER_HPP
#define DEPHEM_TESTPO_TESTPO_FILTER_HPP

#include <stdio.h> // size_t
#include <set>
#include <utility> // pair

#include "TestpoRecord.hpp"
#include "../EphemerisRelease.hpp"

namespace dph { namespace testpo {

// Фильтр записей.
class Filter
{
public:
	// Добавить временной промежуток.
	void addSpan(double startDate, double endDate);

	// Добавить дату.
	void addDate(double date);

	// Добавить искомое тело.
	void addTarget(size_t targets);

    // Добавить центральное тело.
	void addCenter(size_t centers);

	// Добавить компоненту.
	void addComponent(size_t components);

	// Подогнать под выпуск эфемерид:
	void fit(const EphemerisRelease& ephem);

	// Очистить фильтр.
	void clear();

	// Проверка объекта на внютреннюю инициализацию.
	bool empty() const;

	// Проверка объекта записи на её соответствие фильтру.
	bool pass(const dph::testpo::Record& record) const;

private:

	std::set<std::pair<double, double> >	m_spans;		// Временные промежутки.
	std::set<size_t>						m_targets;		// Искомые тела.
	std::set<size_t>						m_centers;		// Центральные тела.
	std::set<size_t>						m_components;	// Компоненты.

}; // class Filter

} } // namespace dph::testpo

void dph::testpo::Filter::addSpan(double startDate, double endDate)
{
    m_spans.insert(std::make_pair(startDate, endDate));
}

void dph::testpo::Filter::addDate(double date)
{
	m_spans.insert(std::make_pair(date, date));
}

void dph::testpo::Filter::addTarget(size_t target)
{
	m_targets.insert(target);
}

void dph::testpo::Filter::addCenter(size_t center)
{
	m_centers.insert(center);
}

void dph::testpo::Filter::addComponent(size_t component)
{
	m_components.insert(component);
}

void dph::testpo::Filter::fit(const dph::EphemerisRelease& ephem)
{
	addSpan(ephem.startDate(), ephem.endDate());
}

void dph::testpo::Filter::clear()
{
	std::set<std::pair<double, double> >().swap(m_spans);
	std::set<size_t>().swap(m_targets);
	std::set<size_t>().swap(m_centers);
	std::set<size_t>().swap(m_components);
}

bool dph::testpo::Filter::empty() const
{
	return m_spans.empty() && 
                m_targets.empty() && 
                    m_centers.empty() && 
                        m_components.empty();
}

bool dph::testpo::Filter::pass(const dph::testpo::Record& record) const
{
	if(empty())
    {
        return true;
    }

    // Проверка промежутков:
	if (m_spans.empty() == false)
	{
		bool pass = false;
		std::set<std::pair<double, double> >::const_iterator span;
		for (span = m_spans.begin(); span != m_spans.end(); ++span)
		{
			if (record.date() >= span->first && record.date() <= span->second)
			{
				pass = true;
				break;
			}
		}

		if (pass == false) return false;
	}

	// Проверка искомых элементов:
	if (m_targets.empty() == false)
	{
		bool pass = false;
		std::set<size_t>::const_iterator target;
		for (target = m_targets.begin(); target != m_targets.end(); ++target)
		{
			if (record.target() == *target)
			{
				pass = true;
				break;
			}
		}

		if (pass == false) return false;
	}
		
	// Проверка центральных элементов:
	if (m_centers.empty() == false)
	{
		bool pass = false;
		std::set<size_t>::const_iterator center;
		for (center = m_centers.begin(); center != m_centers.end(); ++center)
		{
			if (record.center() == *center)
			{
				pass = true;
				break;
			}
		}
			
		if (pass == false) return false;
	}

	// Проверка компонент:
	if (m_components.empty() == false)
	{
		bool pass = false;
		std::set<size_t>::const_iterator componentNumber;
		for (componentNumber = m_components.begin(); componentNumber != m_components.end(); 
			++componentNumber)
		{
			if (record.component() == *componentNumber)
			{
				pass = true;
				break;
			}
		}

		if (pass == false) return false;
	}	

	return true;
}

#endif // DEPHEM_TESTPO_TESTPO_FILTER_HPP