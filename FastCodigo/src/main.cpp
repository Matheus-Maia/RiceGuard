#include <iostream>
#include <fstream>
#include <filesystem>
#include "viab/analise_viabilidade.h"
#include "io/csv_reader.h"
#include "summary/summary_generator.h"
#include <string>
int main(int argc,char**argv){
    if(argc<3){std::cerr<<"Uso: analise <in.csv> <out_dir>\n";return 1;}
    auto in=argv[1]; auto od=argv[2]; std::filesystem::create_directories(od);
    auto dias=io::ler_dados(in);
    std::vector<viab::Fase> fases={
        {"Germinação",10,45,20,35,5,15},
        {"Emergência e estabelecimento da plântula",12,35,25,30,10,20},
        {"Desenvolvimento da raiz",16,35,25,28,14,21},
        {"Alongamento da folha",7,45,31,31,21,35},
        {"Perfilhamento",9,33,25,31,14,28},
        {"Diferenciação do primórdio floral",15,35,25,30,7,14},
        {"Emissão da panícula",15,38,25,28,7,10},
        {"Antese",22,35,30,33,5,7},
        {"Maturação",12,30,20,25,25,35}
    };
    auto R=viab::rodar_analise(dias,fases);
    std::ofstream(std::string(od)+"/analise_detalhada.csv")<<summary::gerar_csv_detalhado(R);
    std::ofstream(std::string(od)+"/resumo_mensal.csv")<<summary::gerar_csv_resumo_mensal(R,dias);
}