#pragma once
#include <vector>
#include <string>
#include "dia.h"
#include "fase.h"
namespace viab {
bool dentro(double x, double min, double max);
bool proxima_combinacao(std::vector<int>& estado, const std::vector<Fase>& fases);
struct ResultadoData {
    std::string data_str;
    double prob_viabilidade=0.0;
    double rendimento_medio=0.0;
    double prob_esbranquiamento=0.0;
    double prob_reducao_moagem=0.0;
    double prob_optimo=0.0;
    long long total_caminhos=0;
    long long caminhos_viaveis=0;
};
std::vector<ResultadoData> rodar_analise(const std::vector<Dia>& dias,const std::vector<Fase>& fases);
} // namespace viab