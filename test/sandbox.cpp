#include <iostream>
#include <dephem/dephem.h>

int main()
{
    using namespace dph;

    DevelopmentEphemeris de("D:/Linux/de441/linux_m13000p17000.441");

    std::cout << de.bodyGms() << std::endl;

    return 0;
}

