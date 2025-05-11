#pragma once
#include <vector>
#include <string>
#include "../viab/dia.h"
namespace model::io {
std::vector<viab::Dia> ler_dados(const std::string& filepath);
}