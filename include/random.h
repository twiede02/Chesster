#pragma once

#include <chrono>
#include <random> 

std::mt19937 gen(static_cast<unsigned int>(
    std::chrono::system_clock::now().time_since_epoch().count()));
std::uniform_int_distribution<> dis(1, 1000);

