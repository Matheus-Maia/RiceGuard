#include "viab/analise_viabilidade.h"
#include "io/csv_reader.h"  
#include "summary/summary_generator.h"  
#include <gtest/gtest.h>

using namespace viab;
using namespace io;  // Namespace para ler_dados
using namespace std;  

TEST(AnaliseTest, TempOtimaIdentica) {
    // Cria um Dia sempre dentro do ótimo
    Dia d{"01/01/2025",0,25,25};
    Fase f{"X",20,30,22,28,1,1};
    std::vector<Dia> dias{d};
    std::vector<Fase> fases{f};
    auto res = rodar_analise(dias,fases);
    EXPECT_EQ(res.size(),1);
    EXPECT_DOUBLE_EQ(res[0].prob_optimo,1.0);
}

TEST(AnaliseTest, TempForaOtima) {
    Dia d{"01/01/2025",0,35,35};
    Fase f{"X",10,40,20,30,1,1};
    auto res = rodar_analise({d},{f});
    EXPECT_EQ(res[0].prob_optimo,0.0);
}

//1. Testes para Funções Utilitárias (dentro e proxima_combinacao)

TEST(UtilsTest, FuncaoDentroDoIntervalo) {
    EXPECT_TRUE(viab::dentro(5.0, 4.0, 6.0));
    EXPECT_FALSE(viab::dentro(3.9, 4.0, 6.0));
    EXPECT_FALSE(viab::dentro(6.1, 4.0, 6.0));
    EXPECT_TRUE(viab::dentro(4.0, 4.0, 6.0)); // Borda inferior
    EXPECT_TRUE(viab::dentro(6.0, 4.0, 6.0)); // Borda superior
}

TEST(UtilsTest, GeracaoCombinacoes) {
    std::vector<Fase> fases = {
        {"F1", 0,0,0,0, 2,3}, // durMin=2, durMax=3
        {"F2", 0,0,0,0, 1,2}
    };
    
    std::vector<int> combo = {2,1};
    int count = 0;
    do {
        count++;
    } while(viab::proxima_combinacao(combo, fases));
    
    EXPECT_EQ(count, (3-2+1)*(2-1+1)); // 2*2=4 combinações
}
//2. Testes de Leitura de Dados (CSV Reader)

TEST(CSVReaderTest, LeituraArquivoValido) {
    auto dias = io::ler_dados("../test_data/valid.csv");
    ASSERT_FALSE(dias.empty());
    EXPECT_EQ(dias[0].mes, 1);
}

//3. Testes de Fases e Combinações

TEST(AnaliseTest, FasesSobrepostas) {
    std::vector<Dia> dias(100, {"01/01/2024",0,25.0, 22.0}); // Tmin ajustado para 22 (dentro de F1)
    std::vector<Fase> fases = {
        {"F1", 20,30, 22,28, 5,5},  // Tmin de sobrevivência: 20
        {"F2", 18,28, 20,26, 5,5}   // Tmin de sobrevivência: 18
    };
    
    auto resultados = viab::rodar_analise(dias, fases);
    ASSERT_EQ(resultados.size(), 100);
    EXPECT_NEAR(resultados[0].prob_viabilidade, 1.0, 0.001);
}

TEST(AnaliseTest, CombinacaoInvalidaPorDadosInsuficientes) {
    std::vector<Dia> dias(10, {"01/01/2024",0,25.0,15.0});
    std::vector<Fase> fases = {
        {"F1", 20,30, 22,28, 5,5},
        {"F2", 18,28, 20,26, 5,5}
    };
    
    auto resultados = viab::rodar_analise(dias, fases);
    EXPECT_EQ(resultados[0].caminhos_viaveis, 0);
}

//4. Testes de Penalidades e Riscos
TEST(RiscosTest, FaseMaturacaoForaRisco) {
    std::vector<Dia> dias(10, {"01/01",0,25.0, 20.0});
    Fase fase{"Maturação",0,50,0,50,5,5};
    auto res = viab::rodar_analise(dias, {fase});
    EXPECT_DOUBLE_EQ(res[0].prob_esbranquiamento, 0.0);
    EXPECT_DOUBLE_EQ(res[0].prob_reducao_moagem, 0.0);
}

TEST(AnaliseTest, TemperaturasNoLimite) {
    Dia d{"01/01",0,30.0, 20.0}; // Tmax=30 (limite), Tmin=20 (limite)
    Fase f{"F1",20,30,20,30,1,1};
    auto res = viab::rodar_analise({d}, {f});
    EXPECT_DOUBLE_EQ(res[0].prob_optimo, 1.0);
}

TEST(RiscosTest, PenalidadeZeraYield) {
    Dia d{"01/01",0,50.0, 40.0}; // Penalidades altas
    Fase f{"F1",0,100,0,100,1,1};
    auto res = viab::rodar_analise({d}, {f});
    EXPECT_DOUBLE_EQ(res[0].rendimento_medio, 0.0);
}

TEST(EdgeCasesTest, CombinacaoExcedeDiasDisponiveis) {
    std::vector<Dia> dias(5, {"01/01",0,25,25});
    std::vector<Fase> fases = {{"F1",0,0,0,0,5,5}}; // Requer 5 dias
    auto res = viab::rodar_analise(dias, fases);
    // Apenas a primeira data tem dias suficientes
    EXPECT_GT(res[0].caminhos_viaveis, 0);
    EXPECT_EQ(res[1].caminhos_viaveis, 0); // Data 1 não tem dias suficientes
}

TEST(RiscosTest, PenalidadeMaximaDiaENoite) {
    Dia d{"01/01/2024",0,40.0, 30.0}; 
    std::vector<Fase> fases = {{"F1", 0,50, 0,50, 1,1}};
    
    auto res = viab::rodar_analise({d}, fases);
    double yield_esperado = 0.0; // Devido ao std::max(0.0, ...)
    EXPECT_NEAR(res[0].rendimento_medio, yield_esperado, 0.001);
} 

TEST(RiscosTest, EsbranquiamentoEMoagem) {
    std::vector<Dia> dias(10, {"01/01/2024",0,35.0, 28.0});
    Fase fase{"Maturação", 0,50, 0,50, 5,5}; // Nome correto da fase
    
    auto res = viab::rodar_analise(dias, {fase});
    EXPECT_NEAR(res[0].prob_esbranquiamento, 1.0, 0.001);
    EXPECT_NEAR(res[0].prob_reducao_moagem, 1.0, 0.001);
}

//5. Testes de Saída e Relatórios

TEST(SummaryTest, GeracaoCSVDetalhado) {
    viab::ResultadoData r1{"01/01", 0.5, 0.8, 0.2, 0.1, 0.3, 100, 50};
    std::string csv = summary::gerar_csv_detalhado({r1});
    
    ASSERT_TRUE(csv.find("01/01,0.5,0.8,0.2,0.1,0.3,100,50") != std::string::npos);
}

TEST(SummaryTest, ResumoMensalAgrupamento) {
    std::vector<Dia> dias = {
        {"01/01/2024",0,0,0}, {"02/02/2024",1,0,0}
    };
    std::vector<viab::ResultadoData> resultados = {
        {"01/01", 1.0, 1.0, 0.0, 0.0, 1.0, 0,0},
        {"02/02", 0.5, 0.5, 0.5, 0.5, 0.0, 0,0}
    };
    
    std::string resumo = summary::gerar_csv_resumo_mensal(resultados, dias);
    ASSERT_TRUE(resumo.find("1,1,1,0,0,1") != std::string::npos); // Mês 1
    ASSERT_TRUE(resumo.find("2,0.5,0.5,0.5,0.5,0") != std::string::npos); // Mês 2
}

//6. Testes de Borda e Casos Especiais

TEST(EdgeCasesTest, SemDiasDisponiveis) {
    auto res = viab::rodar_analise({}, {});
    EXPECT_TRUE(res.empty());
}

TEST(EdgeCasesTest, OverflowCombinacoes) {
    std::vector<Fase> fases = {
        {"F1", 0, 0, 0, 0, 1, 1000000}, // Gera 1e6 combinações por fase
        {"F2", 0, 0, 0, 0, 1, 1000000}
    };

    // Crie um vector<Dia> explicitamente
    std::vector<Dia> dias = {
        {"01/01/2024",0,0,0}, {"02/02/2024",1,0,0}
    };
    
    EXPECT_THROW({
        try {
            viab::rodar_analise(dias, fases); // Agora passa o vector já construído
        } catch (const std::overflow_error& e) {
            throw;
        } catch (...) {
            FAIL() << "Exceção diferente esperada";
        }
    }, std::overflow_error);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}