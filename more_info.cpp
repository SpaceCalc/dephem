#include "more_info.h"

std::string dph::more_info::releaseLabel(const EphemerisRelease& ephemerisRelease)
{
	if (ephemerisRelease.m_ready == false)
	{
		return std::string();
	}
	else
	{
		return ephemerisRelease.m_releaseLabel;
	}
}
