#pragma once
#include <vector>
#include <string>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include "dia.h"
#include "fase.h"

namespace model::viab {
bool dentro(double x, double min, double max);
bool proxima_combinacao(std::vector<int>& estado, const std::vector<Fase>& fases);

/**
 * @brief Gera uma combinação específica de durações de fases a partir de um índice
 * 
 * @param combinacao Vector que receberá a combinação gerada
 * @param fases Vector com as fases e suas restrições de duração
 * @param indice Valor numérico que identifica a combinação específica a gerar
 */
void gerar_combinacao_por_indice(std::vector<int>& combinacao,
                                const std::vector<Fase>& fases,
                                long long indice);

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

std::vector<ResultadoData> rodar_analise(const std::vector<Dia>& dias, const std::vector<Fase>& fases);
} // namespace model::viab