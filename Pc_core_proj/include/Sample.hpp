#pragma once

#include <cstdint>

struct Sample
{
    std::uint32_t time_ms{};
    int x_mg{};
    int y_mg{};
    int z_mg{};
};