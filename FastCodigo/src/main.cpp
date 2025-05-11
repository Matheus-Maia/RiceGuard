#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "model/viab/analise_viabilidade.h"
#include "model/io/csv_reader.h"
#include "model/io/json_loader.h"
#include "model/summary/summary_generator.h"

namespace fs = std::filesystem;
/**
 * @brief Ponto de entrada para análise de viabilidade do arroz (Oryza sativa L.).
 * 
 * Fluxo:
 *  1) Valida argumentos
 *  2) Carrega CSV de dados meteorológicos
 *  3) Configura fases fenológicas (ideal carregar de JSON)
 *  4) Executa análise de viabilidade
 *  5) Gera relatórios CSV de saída
 * 
 * @param argc Deve ser 3: [programa, arquivo_entrada.csv, pasta_saida]
 * @param argv Caminhos de entrada/saída
 * @return int 0 = sucesso, 1 = erro de argumento/arquivo, 2 = erro desconhecido
 */
int main(int argc,char**argv){
    try {
        // ======================================
        // 1. Validação de Entrada
        // ======================================
        if (argc != 3) {
            throw std::invalid_argument(
                "Uso correto: " + std::string(argv[0]) +
                " <arquivo_entrada.csv> <pasta_saida>"
            );
        }

        const fs::path caminho_entrada(argv[1]);
        const fs::path pasta_saida(argv[2]);
        std::string caminho_json = "/home/yuka/Desktop/faculdade/PM/Codigo/FastCodigo/src/config/fases_cultivo_arroz.json";

        if (!fs::exists(caminho_entrada)) {
            throw std::runtime_error("Arquivo de entrada não encontrado: " 
                                     + caminho_entrada.string());
        }

        if (!fs::exists(caminho_json)) {
            throw std::runtime_error("Arquivo json não encontrado!");
        }

        // ======================================
        // 2. Carregamento de Dados
        // ======================================
        // Leitura robusta do CSV, com exceções específicas em caso de falha

        const auto dados_meteorologicos = model::io::ler_dados(caminho_entrada.string());

         // ======================================
        // 3. Configuração de Fases (Fenologia)
        // ======================================
        
        const auto fases = model::io::carregar_fases(caminho_json);

        // ======================================
        // 4. Processamento Principal
        // ======================================

        const auto Resultado = model::viab::rodar_analise(dados_meteorologicos,fases);

        // ======================================
        // 5. Geração de Relatórios
        // ======================================
        fs::create_directories(pasta_saida);

        // 5.1 CSV Detalhado
        std::ofstream(std::string(pasta_saida)+"/analise_detalhada.csv")<<model::summary::gerar_csv_detalhado(Resultado);

        // 5.2 CSV Resumo Mensal
        std::ofstream(std::string(pasta_saida)+"/resumo_mensal.csv")<<model::summary::gerar_csv_resumo_mensal(Resultado,dados_meteorologicos);

        return 0;
    }
    catch (const std::invalid_argument& ia) {
        std::cerr << "Argumento inválido: " << ia.what() << "\n";
        return 1;
    }
    catch (const std::runtime_error& re) {
        std::cerr << "Erro de I/O: " << re.what() << "\n";
        return 1;
    }
    catch (const std::exception& ex) {
        std::cerr << "Erro: " << ex.what() << "\n";
        return 1;
    }
    catch (...) {
        std::cerr << "Erro desconhecido.\n";
        return 2;
    }
}
