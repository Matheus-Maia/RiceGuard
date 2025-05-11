#pragma once
#include <vector>
#include <string>
#include "../viab/fase.h"

namespace model::io {
    std::vector<viab::Fase> carregar_fases(const std::string& caminho_arquivo);
} // namespace model::io
namespace model::io {
    std::vector<viab::Fase> carregar_fases(const std::string& caminho_json);
}