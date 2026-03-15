#include "SerialOutputPrinter.hpp"

#include <iostream>

void SerialOutputPrinter::print(const Sample& sample) const
{
    std::cout
        << "time_ms=" << sample.time_ms
        << " | x_mg=" << sample.x_mg
        << " | y_mg=" << sample.y_mg
        << " | z_mg=" << sample.z_mg
        << std::endl;
}
