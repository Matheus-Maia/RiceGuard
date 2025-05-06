# helpers/posprocessamento.py
import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def gerar_relatorios(resumo_path, detalhe_path, output_dir):
    """
    Lê os arquivos CSV gerados pelo C++ e cria gráficos.
    """
    print("Iniciando pós-processamento e geração de relatórios...")
    # Criar pasta de relatórios
    os.makedirs(output_dir, exist_ok=True)
    print(f"Diretório de relatórios '{output_dir}' assegurado.")

    # --- Leitura dos Arquivos ---
    try:
        # Ler resumo mensal (gerado pelo C++)
        resumo = pd.read_csv(resumo_path, decimal='.') # C++ usa '.' como decimal
        print(f"Arquivo de resumo lido: {resumo_path}")
        # Ler análise detalhada (opcional para alguns gráficos, mas bom ter)
        detalhe = pd.read_csv(detalhe_path, decimal='.') # C++ usa '.' como decimal
        detalhe['Data'] = pd.to_datetime(detalhe['Data'], format='%d/%m/%Y')
        detalhe['Mês'] = detalhe['Data'].dt.month
        print(f"Arquivo detalhado lido: {detalhe_path}")

    except FileNotFoundError as e:
        print(f"Erro: Arquivo de entrada não encontrado: {e.filename}")
        sys.exit(1)
    except Exception as e:
        print(f"Erro ao ler os arquivos CSV de resultados: {e}")
        sys.exit(1)

    # Validar colunas esperadas no resumo
    expected_resumo_cols = [
        'Mês', 'probabilidade_viabilidade_media', 'rendimento_medio',
        'prob_esbranquiamento_media', 'prob_reducao_moagem_media'
    ]
    missing_resumo_cols = [col for col in expected_resumo_cols if col not in resumo.columns]
    if missing_resumo_cols:
        print(f"Erro: Colunas faltando no arquivo de resumo '{resumo_path}': {missing_resumo_cols}")
        print(f"Colunas encontradas: {resumo.columns.tolist()}")
        sys.exit(1)

    # Ordenar resumo por mês para os gráficos ficarem corretos
    resumo = resumo.sort_values(by='Mês').reset_index(drop=True)

    # --- Geração de Gráficos ---
    print("Gerando gráficos...")
    plt.style.use('seaborn-v0_8-darkgrid') # Estilo um pouco mais moderno

    try:
        # Gráfico de Viabilidade Média
        plt.figure(figsize=(10, 6))
        plt.bar(resumo['Mês'], resumo['probabilidade_viabilidade_media'], color='skyblue')
        plt.title('Probabilidade Média de Viabilidade por Mês de Plantio')
        plt.xlabel('Mês de Plantio')
        plt.ylabel('Probabilidade Média')
        plt.xticks(resumo['Mês']) # Garante que todos os meses presentes sejam mostrados
        plt.ylim(0, 1) # Probabilidade vai de 0 a 1
        plt.tight_layout()
        plot_path = os.path.join(output_dir, 'viabilidade_mensal.png')
        plt.savefig(plot_path)
        plt.close()
        print(f"Gráfico salvo: {plot_path}")

        # Gráfico de Rendimento Médio
        plt.figure(figsize=(10, 6))
        plt.bar(resumo['Mês'], resumo['rendimento_medio'], color='lightcoral')
        plt.title('Rendimento Médio Esperado por Mês de Plantio')
        plt.xlabel('Mês de Plantio')
        plt.ylabel('Fator de Rendimento Médio (0-1)')
        plt.xticks(resumo['Mês'])
        plt.ylim(0, 1) # Rendimento relativo vai de 0 a 1
        plt.tight_layout()
        plot_path = os.path.join(output_dir, 'rendimento_mensal.png')
        plt.savefig(plot_path)
        plt.close()
        print(f"Gráfico salvo: {plot_path}")

        # Gráfico de Riscos Médios
        plt.figure(figsize=(10, 6))
        plt.plot(resumo['Mês'], resumo['prob_esbranquiamento_media'],
                 marker='o', linestyle='-', color='darkorange', label='Esbranquiamento (Tmax > 30°C)')
        plt.plot(resumo['Mês'], resumo['prob_reducao_moagem_media'],
                 marker='s', linestyle='--', color='darkviolet', label='Redução Moagem (Tmin > 27°C)')
        plt.title('Probabilidade Média de Riscos de Qualidade por Mês de Plantio')
        plt.xlabel('Mês de Plantio')
        plt.ylabel('Probabilidade Média')
        plt.xticks(resumo['Mês'])
        plt.ylim(0, 1) # Probabilidade vai de 0 a 1
        plt.legend(title="Risco Associado")
        plt.grid(True, which='both', linestyle='--', linewidth=0.5)
        plt.tight_layout()
        plot_path = os.path.join(output_dir, 'riscos_mensais.png')
        plt.savefig(plot_path)
        plt.close()
        print(f"Gráfico salvo: {plot_path}")

        # (Opcional) Gráfico da evolução diária da viabilidade (usando 'detalhe')
        plt.figure(figsize=(14, 7))
        plt.plot(detalhe['Data'], detalhe['probabilidade_viabilidade'],
                 label='Prob. Viabilidade Diária', color='green', alpha=0.7)
        # Adicionar média móvel para suavizar
        detalhe['viab_media_movel_30d'] = detalhe['probabilidade_viabilidade'].rolling(window=30, center=True, min_periods=1).mean()
        plt.plot(detalhe['Data'], detalhe['viab_media_movel_30d'],
                 label='Média Móvel 30d', color='darkgreen', linewidth=2)
        plt.title('Probabilidade de Viabilidade Diária e Média Móvel (30 dias)')
        plt.xlabel('Data de Plantio')
        plt.ylabel('Probabilidade')
        plt.ylim(0, 1)
        plt.legend()
        plt.grid(True, which='both', linestyle='--', linewidth=0.5)
        plt.tight_layout()
        plot_path = os.path.join(output_dir, 'viabilidade_diaria_evolucao.png')
        plt.savefig(plot_path)
        plt.close()
        print(f"Gráfico salvo: {plot_path}")


    except Exception as e:
        print(f"Erro durante a geração dos gráficos: {e}")
        # Não sai do script, mas avisa do erro
    finally:
        plt.close('all') # Garante que todas as figuras sejam fechadas

    print("Geração de relatórios concluída.")


if __name__ == '__main__':
     # --- Configuração dos Caminhos ---
    script_dir = os.path.dirname(__file__)
    project_root = os.path.dirname(script_dir)

    default_resumo = os.path.join(project_root, 'processados', 'resumo_mensal_cpp.csv')
    default_detalhe = os.path.join(project_root, 'processados', 'analise_detalhada.csv')
    default_output_dir = os.path.join(project_root, 'relatorios')

    resumo_file = sys.argv[1] if len(sys.argv) > 1 else default_resumo
    detalhe_file = sys.argv[2] if len(sys.argv) > 2 else default_detalhe
    output_directory = sys.argv[3] if len(sys.argv) > 3 else default_output_dir
    # --- Fim Configuração ---

    if not os.path.exists(resumo_file):
         print(f"Erro: Arquivo de resumo '{resumo_file}' não encontrado.")
         print("Uso: python helpers/posprocessamento.py [resumo.csv] [detalhe.csv] [diretorio_relatorios]")
         sys.exit(1)
    if not os.path.exists(detalhe_file):
        print(f"Erro: Arquivo detalhado '{detalhe_file}' não encontrado.")
        print("Uso: python helpers/posprocessamento.py [resumo.csv] [detalhe.csv] [diretorio_relatorios]")
        sys.exit(1)

    gerar_relatorios(resumo_file, detalhe_file, output_directory)
    print("Pós-processamento concluído.")