#include "CsvParser.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace
{
std::string trim(const std::string& input)
{
    const auto first = std::find_if_not(input.begin(), input.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });

    const auto last = std::find_if_not(input.rbegin(), input.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();

    if (first >= last)
    {
        return {};
    }

    return std::string(first, last);
}

std::vector<std::string> splitCsv(const std::string& line)
{
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        parts.push_back(trim(item));
    }

    return parts;
}
} // namespace

std::optional<Sample> CsvParser::parseLine(const std::string& line) const
{
    const std::string cleaned = trim(line);
    if (cleaned.empty() || isHeader(cleaned))
    {
        return std::nullopt;
    }

    const auto parts = splitCsv(cleaned);
    if (parts.size() != 4)
    {
        return std::nullopt;
    }

    try
    {
        Sample sample;
        sample.time_ms = static_cast<std::uint32_t>(std::stoul(parts[0]));
        sample.x_mg = std::stoi(parts[1]);
        sample.y_mg = std::stoi(parts[2]);
        sample.z_mg = std::stoi(parts[3]);
        return sample;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

bool CsvParser::isHeader(const std::string& line)
{
    return line.rfind("time_ms", 0) == 0;
}