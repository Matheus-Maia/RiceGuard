# Rice Thermal Viability Analyzer 🌾🌡️  
Projeto pessoal desenvolvido em **C++** com o objetivo de explorar como a temperatura impacta o cultivo de arroz ao longo das fases fenológicas.  

A proposta é apresentar visualmente — e de forma baseada em dados — como mudanças térmicas podem afetar a viabilidade do plantio. Embora ainda em fase experimental, o projeto busca unir ciência, código e curiosidade.

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)  

---

## 🚀 Principais Recursos  
- **Modelagem térmica** baseada em dados reais para análises precisas;  
- **Geração de relatórios CSV** com probabilidades de risco detalhadas;  
- **Desempenho otimizado** com paralelismo via OpenMP para processamento rápido.  

---

## 🎛️ Estratégias de Imputação de Dados  
Durante o pré-processamento, quatro estratégias estão disponíveis para lidar com valores faltantes nos dados climáticos.  

### 1. **Interpolação**
Aplica interpolação linear ao longo do tempo para estimar valores faltantes com base nos vizinhos adjacentes. Após isso:  
- Executa **forward fill** para preencher `NaN` iniciais com o primeiro valor válido;  
- Executa **backward fill** para preencher `NaN` finais com o último valor válido.  

**Quando usar:**  
Ideal para dados esparsamente faltantes e com variação suave.

---

### 2. **Vizinho**
Substitui `NaN` pela observação anterior (**forward fill**) e, em seguida, aplica **backward fill** para completar os restantes.  

**Quando usar:**  
Simples e direto, mas pode gerar repetições longas em séries com muitas ausências consecutivas.

---

### 3. **Randômica**
Gera valores aleatórios dentro dos limites biológicos (`optMinT`, `optMaxT`) definidos no `fases_cultivo_arroz.json`.  

**Características:**  
- Introduz variabilidade plausível;  
- Representa incerteza de forma controlada.  

---

### 4. **Ideal**
Substitui cada `NaN` pela média dos pontos centrais ideais de todas as fases do cultivo:  
`(optMinT + optMaxT) / 2`  

**Características:**  
- Garante consistência com um único ponto ideal;  
- Ignora variações reais ao longo do tempo.  

---

### 🧠 Comparativo entre Estratégias  
| Estratégia   | Fonte dos valores  | Variabilidade | Indicação principal                     |
|--------------|---------------------|---------------|-----------------------------------------|
| Interpolação | Dados próprios      | Baixa         | Faltas esparsas e comportamento suave   |
| Vizinho      | Dados próprios      | Nenhuma       | Simplicidade e consistência             |
| Randômica    | Limites biológicos  | Alta          | Simulação com incerteza controlada      |
| Ideal        | Pontos ideais       | Nenhuma       | Consistência ideal e padronizada        |

---

## ⚙️ Fluxo de Trabalho  

1. **Pré-processamento (Python):**  
   - Aplicação da estratégia de imputação;  
   - Geração do CSV limpo.  

2. **Análise de Dados (C++):**  
   - Processamento de alta performance com OpenMP;  
   - Resultados exportados como CSV.  

3. **Pós-processamento (Python):**  
   - Geração de gráficos e relatórios a partir dos resultados.  

---

## 📂 Estrutura do Projeto  

| Pasta/Arquivo               | Descrição                                      |
|----------------------------|------------------------------------------------|
| `dados_ano.csv`            | Dados climáticos brutos                        |
| `preprocessed/`            | Dados limpos e prontos para análise            |
| `FastCodigo/build/`        | Executável compilado em C++                    |
| `processados/`             | Resultados numéricos da análise                |
| `relatorios/`              | Relatórios visuais e gráficos finais           |

---

## 🛠️ Como Executar  

1. Certifique-se de ter as dependências instaladas;  
2. Escolha uma estratégia de imputação quando solicitado;  
3. O workflow será executado automaticamente.

```bash
python main_workflow.py
```

---

## 📝 Licença  
Este projeto é disponibilizado sob a licença MIT. Para mais detalhes, consulte [LICENSE](LICENSE).
