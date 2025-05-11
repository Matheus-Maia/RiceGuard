#include "json_loader.h"
#include <fstream>
#include "include/external/nlohmann/json.hpp"
#include <stdexcept>

namespace model::io {
    std::vector<viab::Fase> carregar_fases(const std::string& caminho_arquivo) {
        std::vector<viab::Fase> fases;

        try {
            // Abrir o arquivo JSON
            std::ifstream file(caminho_arquivo);
            if (!file.is_open()) {
                throw std::runtime_error("Não foi possível abrir o arquivo: " + caminho_arquivo);
            }

            // Parsear o JSON
            nlohmann::json json_data;
            file >> json_data;

            // Verificar se o JSON contém o array "fases"
            if (!json_data.contains("fases") || !json_data["fases"].is_array()) {
                throw std::runtime_error("JSON inválido: faltando array 'fases'");
            }

            // Processar cada fase do array
            for (const auto& fase_json : json_data["fases"]) {
                // Verificar se todos os campos necessários existem
                if (!fase_json.contains("nome") || !fase_json.contains("minT") ||
                    !fase_json.contains("maxT") || !fase_json.contains("optMinT") ||
                    !fase_json.contains("optMaxT") || !fase_json.contains("durMin") ||
                    !fase_json.contains("durMax")) {
                    throw std::runtime_error("JSON inválido: campo obrigatório faltando em uma fase");
                }

                // Criar um objeto Fase usando o construtor
                fases.emplace_back(
                    fase_json["nome"].get<std::string>(),
                    fase_json["minT"].get<double>(),
                    fase_json["maxT"].get<double>(),
                    fase_json["optMinT"].get<double>(),
                    fase_json["optMaxT"].get<double>(),
                    fase_json["durMin"].get<int>(),
                    fase_json["durMax"].get<int>()
                );
            }

            return fases;
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Erro ao processar o JSON: " + std::string(e.what()));
        } catch (const std::exception& e) {
            throw std::runtime_error("Erro ao carregar as fases: " + std::string(e.what()));
        }
    }
}