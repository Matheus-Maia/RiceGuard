#include "csv_reader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace model::io {

std::vector<viab::Dia> ler_dados(const std::string& caminho_arquivo) {
    std::vector<viab::Dia> dados;
    std::ifstream arquivo(caminho_arquivo);
    
    if (!arquivo.is_open()) {
        throw std::runtime_error("Não foi possível abrir o arquivo: " + caminho_arquivo);
    }
    
    std::string linha;
    // Pular a linha de cabeçalho (Data;Tmax;Tmin)
    std::getline(arquivo, linha);
    
    while (std::getline(arquivo, linha)) {
        std::istringstream ss(linha);
        std::string data_str, tmax_str, tmin_str;
        viab::Dia dia;
        
        // Formato esperado: Data;Tmax;Tmin
        // Exemplo: 01/01/2023;33.4;28.2
        
        // Lê a data
        if (!std::getline(ss, data_str, ';')) {
            throw std::runtime_error("Erro ao ler a data no CSV");
        }
        dia.data_str = data_str;
        
        // Extrai o mês da data (assume formato DD/MM/YYYY)
        try {
            dia.mes = std::stoi(data_str.substr(3, 2));
            if (dia.mes < 1 || dia.mes > 12) {
                throw std::runtime_error("Mês inválido na data: " + data_str);
            }
        } catch (...) {
            throw std::runtime_error("Erro ao extrair mês da data: " + data_str);
        }
        
        // Lê Tmax
        if (!std::getline(ss, tmax_str, ';')) {
            throw std::runtime_error("Erro ao ler a temperatura máxima no CSV");
        }
        try {
            dia.tmax = std::stod(tmax_str);
        } catch (...) {
            throw std::runtime_error("Temperatura máxima inválida: " + tmax_str);
        }
        
        // Lê Tmin
        if (!std::getline(ss, tmin_str)) {
            throw std::runtime_error("Erro ao ler a temperatura mínima no CSV");
        }
        try {
            dia.tmin = std::stod(tmin_str);
        } catch (...) {
            throw std::runtime_error("Temperatura mínima inválida: " + tmin_str);
        }
        
        // Validações adicionais
        if (dia.tmin > dia.tmax) {
            throw std::runtime_error("Temperatura mínima maior que máxima na data: " + data_str);
        }
        
        if (dia.tmax < -50 || dia.tmax > 60 || dia.tmin < -50 || dia.tmin > 60) {
            throw std::runtime_error("Temperatura fora do intervalo válido na data: " + data_str);
        }
        
        dados.push_back(dia);
    }
    
    if (dados.empty()) {
        throw std::runtime_error("Nenhum dado válido encontrado no arquivo CSV");
    }
    
    return dados;
}

} // namespace model::io