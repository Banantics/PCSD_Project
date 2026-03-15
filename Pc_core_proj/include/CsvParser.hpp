#pragma once

#include <optional>
#include <string>

#include "Sample.hpp"

class CsvParser
{
public:
    std::optional<Sample> parseLine(const std::string& line) const;

private:
    static bool isHeader(const std::string& line);
};