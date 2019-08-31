#ifndef DEPHEM_RMANIP_HPP
#define DEPHEM_RMANIP_HPP

#include "lnx/copycut.hpp"

namespace dph { namespace rmanip {

struct LnxCutParameters
{
    std::string baseLnxFilePath;
    std::string newLnxFilePath;
    double startDate;
    double endDate;
};

void CopyCut(const LnxCutParameters& params)
{
    lnx::CopyCutLnxRelease(params.baseLnxFilePath, params.newLnxFilePath, 
        params.startDate, params.endDate);
}


} } // namespace dph::rmanip

#endif // DEPHEM_RELEASEMAINP_HPP