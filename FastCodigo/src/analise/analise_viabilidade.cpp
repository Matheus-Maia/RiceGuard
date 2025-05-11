#include "../model/viab/analise_viabilidade.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <omp.h>
#include <stdexcept>
#include <limits>

namespace model::viab {

// Constantes e configurações para análise de viabilidade
struct AnalysisConfig {
    static constexpr long long LIMITE_COMBINACOES = 100000000;
    static constexpr double TMAX_PEN_THR = 31.0;  // Limite para penalidade diurna
    static constexpr double TMIN_PEN_THR = 21.0;  // Limite para penalidade noturna
    static constexpr double ESBRANQ_THR   = 30.0;  // Limiar de esbranquiamento
    static constexpr double RED_THR       = 27.0;  // Limiar de redução de moagem
    static constexpr double PENAL_DIURNA  = 0.06;
    static constexpr double PENAL_NOTURNA = 0.10;
};

// Verifica intervalo fechado [lo, hi]
bool dentro(double x, double lo, double hi) {
    return x >= lo && x <= hi;
}

// Gera próxima combinação de durações dado vetor atual e restrições
bool proxima_combinacao(std::vector<int>& duracoes, const std::vector<Fase>& fases) {
    for (int i = static_cast<int>(duracoes.size()) - 1; i >= 0; --i) {
        if (duracoes[i] + 1 <= fases[i].durMax) {
            duracoes[i]++;
            for (size_t j = i + 1; j < duracoes.size(); ++j)
                duracoes[j] = fases[j].durMin;
            return true;
        }
    }
    return false;
}

// Estrutura para resultado diário detalhado
struct ResultadoDia {
    bool viavel           = false;
    bool ideal            = false;
    double rendimento     = 0.0;
    bool risco_esbranq    = false;
    bool risco_reducao    = false;
    double penalidade_dia = 0.0;
    double penalidade_noite = 0.0;
};

// Avalia um único dia para uma fase específica
static ResultadoDia avaliar_dia(const Dia& dia,
                                const Fase& fase,
                                const AnalysisConfig& cfg) {
    ResultadoDia res;
    // 1. Viabilidade básica
    res.viavel = dentro(dia.tmax, fase.minT, fase.maxT) &&
                 dentro(dia.tmin, fase.minT, fase.maxT);
    if (!res.viavel)
        return res;
    // 2. Condições ideais
    res.ideal = dentro(dia.tmax, fase.optMinT, fase.optMaxT) &&
                dentro(dia.tmin, fase.optMinT, fase.optMaxT);
    
    // Caso especial: se está em condições ideais, o rendimento é sempre 1.0
    if (res.ideal) {
        res.rendimento = 1.0;
        // 5. Riscos na maturação
        if (fase.nome == "Maturação") {
            res.risco_esbranq = dia.tmax > cfg.ESBRANQ_THR;
            res.risco_reducao = dia.tmin > cfg.RED_THR;
        }
        return res;
    }
    
    // 3. Penalidades (apenas para condições não ideais)
    if (dia.tmax > cfg.TMAX_PEN_THR)
        res.penalidade_dia = (dia.tmax - cfg.TMAX_PEN_THR) * cfg.PENAL_DIURNA;
    if (dia.tmin > cfg.TMIN_PEN_THR)
        res.penalidade_noite = (dia.tmin - cfg.TMIN_PEN_THR) * cfg.PENAL_NOTURNA;
    // 4. Rendimento
    double total_pen = res.penalidade_dia + res.penalidade_noite;
    res.rendimento = std::max(0.0, 1.0 - total_pen);
    // 5. Riscos na maturação
    if (fase.nome == "Maturação") {
        res.risco_esbranq = dia.tmax > cfg.ESBRANQ_THR;
        res.risco_reducao = dia.tmin > cfg.RED_THR;
    }
    return res;
}

// Versão simplificada: único dia, única fase com duração fixa
static ResultadoData analisar_caso_simples(const Dia& dia,
                                          const Fase& fase) {
    ResultadoData out;
    out.data_str           = dia.data_str;
    out.total_caminhos     = 1;
    auto res_dia = avaliar_dia(dia, fase, AnalysisConfig());
    out.caminhos_viaveis    = res_dia.viavel ? 1 : 0;
    out.prob_viabilidade   = res_dia.viavel ? 1.0 : 0.0;
    
    // Calcula o rendimento baseado nas penalidades diárias e noturnas
    if (res_dia.viavel) {
        double total_pen = res_dia.penalidade_dia + res_dia.penalidade_noite;
        out.rendimento_medio = std::max(0.0, 1.0 - total_pen);
    } else {
        out.rendimento_medio = 0.0;
    }
    
    out.prob_optimo        = res_dia.ideal    ? 1.0 : 0.0;
    out.prob_esbranquiamento = res_dia.risco_esbranq ? 1.0 : 0.0;
    out.prob_reducao_moagem  = res_dia.risco_reducao ? 1.0 : 0.0;
    return out;
}

// Avalia sequência completa de dias e fases
static bool avaliar_sequencia(const std::vector<Dia>& dias,
                              size_t inicio,
                              const std::vector<Fase>& fases,
                              const std::vector<int>& duracoes,
                              const AnalysisConfig& cfg,
                              double& penal_dia,
                              double& penal_noite,
                              bool& risco_esb,
                              bool& risco_red,
                              bool& seq_ideal) {
    penal_dia = penal_noite = 0.0;
    risco_esb = risco_red = false;
    seq_ideal = true;
    int total_dias = 0;
    double soma_dia = 0.0, soma_noite = 0.0;
    for (size_t i = 0; i < fases.size(); ++i) {
        const auto& fase = fases[i];
        for (int d = 0; d < duracoes[i]; ++d) {
            size_t idx = inicio + total_dias;
            if (idx >= dias.size()) return false;
            auto res = avaliar_dia(dias[idx], fase, cfg);
            if (!res.viavel) return false;
            seq_ideal &= res.ideal;
            risco_esb |= res.risco_esbranq;
            risco_red |= res.risco_reducao;
            soma_dia += res.penalidade_dia;
            soma_noite += res.penalidade_noite;
            total_dias++;
        }
    }
    if (total_dias > 0) {
        penal_dia   = soma_dia / total_dias;
        penal_noite = soma_noite / total_dias;
    }
    return true;
}


// Mapeia índice para combinação de durações (sistema posicional)
void gerar_combinacao_por_indice(std::vector<int>& comb,
                                const std::vector<Fase>& fases,
                                long long idx) {
    for (int i = fases.size() - 1; i >= 0; --i) {
        int op = fases[i].durMax - fases[i].durMin + 1;
        comb[i] = fases[i].durMin + (idx % op);
        idx /= op;
    }
}

// Função principal: executa análise para cada dia inicial
std::vector<ResultadoData> rodar_analise(const std::vector<Dia>& dias,
                                         const std::vector<Fase>& fases) {
    std::vector<ResultadoData> resultados;
    size_t n = dias.size();
    if (n == 0 || fases.empty()) return resultados;
    // Verifica consistência de fases
    for (auto& f : fases)
        if (f.durMin > f.durMax)
            throw std::invalid_argument("DurMin > DurMax em fase: " + f.nome);
    
    // Caso simplificado: Um único dia e uma única fase
    if (n == 1 && fases.size() == 1) {
        resultados.push_back(analisar_caso_simples(dias[0], fases[0]));
        return resultados;
    }
    
    // Calcula total de combinações e verifica se precisamos de amostragem
    bool precisa_amostragem = false;
    long long total_comb_real = 1;
    
    for (auto& f : fases) {
        long long op = static_cast<long long>(f.durMax - f.durMin + 1);
        
        // Verificamos se excede o limite definido - ativa amostragem em vez de lançar erro
        if (total_comb_real > AnalysisConfig::LIMITE_COMBINACOES / op) {
            precisa_amostragem = true;
            break;
        }
        
        // Verificamos se haverá overflow no cálculo - ativa amostragem em vez de lançar erro
        if (total_comb_real > std::numeric_limits<long long>::max() / op) {
            precisa_amostragem = true;
            break;
        }
        
        total_comb_real *= op;
    }
    
    // Se precisamos de amostragem, limita o número de combinações para algo gerenciável
    if (precisa_amostragem) {
        // Define um valor grande mas abaixo do limite para total_comb_real
        total_comb_real = AnalysisConfig::LIMITE_COMBINACOES;
    }
    
    // Define tamanho da amostra e modo de amostragem
    bool usar_amostragem = precisa_amostragem || 
                          (total_comb_real > AnalysisConfig::LIMITE_COMBINACOES / 10); // Ativamos amostragem quando chega a 10% do limite
    long long total_comb = usar_amostragem ? AnalysisConfig::LIMITE_COMBINACOES / 10 : total_comb_real;
    
    // Inicializa gerador de números aleatórios se for usar amostragem
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<long long> distrib(0, total_comb_real - 1);
    
    // Indicador de progresso
    std::cout << "Iniciando análise de " << n << " dias com " 
              << (usar_amostragem ? "amostragem" : "análise completa") << std::endl;
    if (usar_amostragem) {
        std::cout << "Número total de combinações muito alto: " << total_comb_real 
                  << " -> Usando " << total_comb << " amostras por dia" << std::endl;
    }
    
    // Contadores atômicos para progresso
    std::atomic<size_t> dias_concluidos{0};
    auto inicio_analise = std::chrono::high_resolution_clock::now();
    
    resultados.resize(n);
    #pragma omp parallel for schedule(dynamic)
    for (size_t dia0 = 0; dia0 < n; ++dia0) {
        // Inicializa gerador thread-local para paralelismo
        std::mt19937_64 gen_local = gen;
        gen_local.discard(dia0 * 1000); // Garante sequências diferentes por thread
        
        // pula se não há dias mínimos disponíveis
        int dias_min = 0;
        for (auto& f : fases) dias_min += f.durMin;
        if (static_cast<int>(n - dia0) < dias_min) continue;
        
        long long viaveis = 0;
        double sum_rend = 0.0;
        long long optimos = 0, esb = 0, red = 0;
        std::vector<int> comb(fases.size());
        
        // Define quantas avaliações serão feitas
        long long amostras_avaliadas = 0;
        
        // Amostragem adaptativa (aleatória para muitos casos, exaustiva para poucos)
        if (usar_amostragem) {
            // Modo de amostragem aleatória
            std::uniform_int_distribution<long long> distrib(0, total_comb_real - 1);
            for (long long i = 0; i < total_comb; ++i) {
                long long idx = distrib(gen_local); // Índice aleatório
                gerar_combinacao_por_indice(comb, fases, idx);
                double pd, pn;
                bool r_esb, r_red, seq_id;
                bool ok = avaliar_sequencia(dias, dia0, fases, comb,
                                          AnalysisConfig(), pd, pn,
                                          r_esb, r_red, seq_id);
                amostras_avaliadas++;
                if (!ok) continue;
                viaveis++;
                double rend = std::max(0.0, 1.0 - (pd + pn));
                sum_rend += rend;
                optimos += seq_id;
                esb     += r_esb;
                red     += r_red;
            }
        } else {
            // Modo exaustivo para poucos casos
            for (long long idx = 0; idx < total_comb_real; ++idx) {
                gerar_combinacao_por_indice(comb, fases, idx);
                double pd, pn;
                bool r_esb, r_red, seq_id;
                bool ok = avaliar_sequencia(dias, dia0, fases, comb,
                                          AnalysisConfig(), pd, pn,
                                          r_esb, r_red, seq_id);
                amostras_avaliadas++;
                if (!ok) continue;
                viaveis++;
                double rend = std::max(0.0, 1.0 - (pd + pn));
                sum_rend += rend;
                optimos += seq_id;
                esb     += r_esb;
                red     += r_red;
            }
        }
        
        // Atualiza contadores e mostra progresso
        size_t concluidos = ++dias_concluidos;
        if (concluidos == 1 || concluidos % 10 == 0 || concluidos == n) {
            auto agora = std::chrono::high_resolution_clock::now();
            auto duracao = std::chrono::duration_cast<std::chrono::seconds>(agora - inicio_analise).count();
            
            #pragma omp critical
            {
                double porcentagem = (100.0 * concluidos) / n;
                std::cout << "\rProgresso: " << concluidos << "/" << n 
                          << " dias (" << std::fixed << std::setprecision(1) << porcentagem << "%)";
                
                // Mostra estimativa de tempo restante se já tiver processado pelo menos 5% ou 10 dias
                if ((duracao > 0) && (porcentagem >= 5.0 || concluidos >= 10)) {
                    double dias_por_segundo = static_cast<double>(concluidos) / duracao;
                    double segundos_restantes = (n - concluidos) / dias_por_segundo;
                    
                    int minutos_restantes = static_cast<int>(segundos_restantes / 60);
                    int segundos = static_cast<int>(segundos_restantes) % 60;
                    
                    std::cout << " - Tempo restante estimado: ";
                    if (minutos_restantes > 0) {
                        std::cout << minutos_restantes << " min ";
                    }
                    std::cout << segundos << " seg ";
                }
                
                std::cout << "   " << std::flush;
            }
        }
        
        auto& out = resultados[dia0];
        out.data_str        = dias[dia0].data_str;
        out.total_caminhos  = total_comb_real; // Mostra total real, não amostrado
        
        // Ajusta os resultados com base no modo de amostragem
        if (usar_amostragem && amostras_avaliadas > 0) {
            // Estimativa de caminhos viáveis baseada na proporção da amostra
            double proporcao_viaveis = static_cast<double>(viaveis) / amostras_avaliadas;
            out.caminhos_viaveis = static_cast<long long>(proporcao_viaveis * total_comb_real);
            
            if (viaveis > 0) {
                out.prob_viabilidade    = proporcao_viaveis;
                out.rendimento_medio     = sum_rend / viaveis;
                out.prob_optimo          = static_cast<double>(optimos) / amostras_avaliadas;
                out.prob_esbranquiamento = static_cast<double>(esb)     / viaveis;
                out.prob_reducao_moagem  = static_cast<double>(red)     / viaveis;
            }
        } else if (amostras_avaliadas > 0) {
            // Resultados precisos para casos não amostrados
            out.caminhos_viaveis = viaveis;
            
            if (viaveis > 0) {
                out.prob_viabilidade    = static_cast<double>(viaveis) / total_comb_real;
                out.rendimento_medio     = sum_rend / viaveis;
                out.prob_optimo          = static_cast<double>(optimos) / total_comb_real;
                out.prob_esbranquiamento = static_cast<double>(esb)     / viaveis;
                out.prob_reducao_moagem  = static_cast<double>(red)     / viaveis;
            }
        }
    }
                
                // Mostra tempo total ao finalizar
                auto fim_analise = std::chrono::high_resolution_clock::now();
                auto tempo_total = std::chrono::duration_cast<std::chrono::seconds>(fim_analise - inicio_analise).count();
                std::cout << std::endl << "Análise concluída em " 
              << tempo_total / 60 << " minutos e " 
              << tempo_total % 60 << " segundos." << std::endl;
                
    return resultados;
}

} // namespace model::viab
