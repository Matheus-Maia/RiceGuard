# helpers/preprocessamento.py
import pandas as pd
import os
import sys
from tqdm import tqdm

def preprocessar_dados(input_csv, output_csv):
    """
    Lê o CSV horário, agrega para diário (Tmin, Tmax) e salva
    no formato esperado pelo C++ (Data;Tmax;Tmin).
    """
    print(f"Iniciando pré-processamento de: {input_csv}")
    # Criar pasta de destino se não existir
    output_dir = os.path.dirname(output_csv)
    if output_dir: # Garante que não tente criar se for o diretório atual
        os.makedirs(output_dir, exist_ok=True)
        print(f"Diretório de saída '{output_dir}' assegurado.")

    try:
        df = pd.read_csv(
            input_csv,
            sep=';',
            decimal=',',
            quotechar='"',
            encoding='utf-8-sig',
            dayfirst=True,
            low_memory=False # Pode ajudar com arquivos grandes
        )
        print("Arquivo CSV lido com sucesso.")
    except FileNotFoundError:
        print(f"Erro: Arquivo de entrada não encontrado em '{input_csv}'")
        sys.exit(1)
    except Exception as e:
        print(f"Erro ao ler o CSV: {e}")
        sys.exit(1)

    # --- Validação de Colunas Essenciais ---
    date_col = 'Data'
    time_col = 'Hora (UTC)'
    temp_col = 'Temp. [Hora] (C)' # Ajuste se o nome da coluna for diferente

    required_cols = [date_col, time_col, temp_col]
    missing_cols = [col for col in required_cols if col not in df.columns]
    if missing_cols:
        print(f"Erro: Colunas obrigatórias não encontradas no CSV: {', '.join(missing_cols)}")
        print(f"Colunas disponíveis: {', '.join(df.columns)}")
        sys.exit(1)
    # --- Fim Validação ---

    print("Processando colunas de data e hora...")
    try:
        # Garantir que a hora seja string com 4 dígitos
        df[time_col] = df[time_col].astype(str).str.zfill(4)
        # Combinar data e hora, tratando possíveis erros
        df['datetime'] = pd.to_datetime(
            df[date_col] + ' ' + df[time_col],
            format='%d/%m/%Y %H%M',
            errors='coerce' # Transforma datas inválidas em NaT
        )
        # Remover linhas onde a conversão falhou
        original_rows = len(df)
        df.dropna(subset=['datetime'], inplace=True)
        if len(df) < original_rows:
            print(f"Aviso: {original_rows - len(df)} linhas removidas devido a data/hora inválidas.")

        # Tratar coluna de temperatura
        df[temp_col] = pd.to_numeric(df[temp_col], errors='coerce')
        original_rows = len(df)
        df.dropna(subset=[temp_col], inplace=True)
        if len(df) < original_rows:
            print(f"Aviso: {original_rows - len(df)} linhas removidas devido a valores de temperatura inválidos.")

    except Exception as e:
        print(f"Erro durante o processamento das colunas de data/hora/temperatura: {e}")
        sys.exit(1)

    if df.empty:
        print("Erro: Nenhum dado válido restante após a limpeza inicial.")
        sys.exit(1)

    print("Agregando dados por dia (Tmax, Tmin)...")
    # Usar Grouper para garantir que todos os dias sejam considerados (opcional)
    # diario = df.groupby(pd.Grouper(key='datetime', freq='D')).agg(
    #     Tmax=(temp_col, 'max'),
    #     Tmin=(temp_col, 'min')
    # ).reset_index()

    # Alternativa mais simples (pode pular dias sem dados)
    df['date_only'] = df['datetime'].dt.date
    diario = df.groupby('date_only').agg(
         Tmax=(temp_col, 'max'),
         Tmin=(temp_col, 'min')
    ).reset_index()


    # Remover dias onde Tmax ou Tmin não puderam ser calculados (se houver)
    diario.dropna(subset=['Tmax', 'Tmin'], inplace=True)

    if diario.empty:
        print("Erro: Nenhum dado diário pôde ser agregado.")
        sys.exit(1)

    print("Formatando e salvando dados pré-processados...")
    # Formatar a data como DD/MM/YYYY para o C++ ler
    # Certifique-se que 'date_only' é datetime antes de formatar
    diario['Data'] = pd.to_datetime(diario['date_only']).dt.strftime('%d/%m/%Y')

    # Selecionar e salvar colunas no formato CSV esperado pelo C++
    try:
        diario[['Data', 'Tmax', 'Tmin']].to_csv(
            output_csv,
            index=False,
            sep=';',      # Usar ponto e vírgula como separador
            decimal='.'   # Usar ponto como separador decimal
            )
        print(f"Dados pré-processados salvos com sucesso em: {output_csv}")
    except Exception as e:
        print(f"Erro ao salvar o arquivo pré-processado: {e}")
        sys.exit(1)

if __name__ == '__main__':
    # --- Configuração dos Caminhos ---
    # Assume que o script está em 'helpers/' e o CSV original na raiz
    script_dir = os.path.dirname(__file__)
    project_root = os.path.dirname(script_dir) # Vai para o diretório pai (raiz do projeto)

    default_input = os.path.join(project_root, 'dados_ano.csv')
    default_output = os.path.join(project_root, 'preprocessed', 'dados_diarios.csv')

    # Permite sobrescrever caminhos via argumentos de linha de comando
    input_file = sys.argv[1] if len(sys.argv) > 1 else default_input
    output_file = sys.argv[2] if len(sys.argv) > 2 else default_output
    # --- Fim Configuração ---

    if not os.path.exists(input_file):
         print(f"Erro: Arquivo de entrada '{input_file}' não encontrado.")
         print("Uso: python helpers/preprocessamento.py [arquivo_entrada.csv] [arquivo_saida.csv]")
         sys.exit(1)

    preprocessar_dados(input_file, output_file)
    print("Pré-processamento concluído.")