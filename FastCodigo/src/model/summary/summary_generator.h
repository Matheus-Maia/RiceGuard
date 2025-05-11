#pragma once
#include <vector>
#include <string>
#include "../viab/analise_viabilidade.h"
#include "../viab/dia.h"
namespace model::summary {
std::string gerar_csv_detalhado(const std::vector<viab::ResultadoData>& resultados);
std::string gerar_csv_resumo_mensal(const std::vector<viab::ResultadoData>& resultados,const std::vector<viab::Dia>& dias);
}