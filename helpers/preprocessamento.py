import pandas as pd
import os
import sys
import random
import json
import argparse
from tqdm import tqdm

# Caminho fixo para o JSON de fases, relativo a este script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_FASES_JSON = os.path.join(SCRIPT_DIR, '../FastCodigo/config/fases_cultivo_arroz.json')

# Carrega as fases de cultivo (para imputação randômica/opção ideal)
def carregar_fases(json_path=DEFAULT_FASES_JSON):
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    return data['fases']

# Estratégias de imputação
def imputar_interpolacao(df, col='temperatura'):
    return (
        df.set_index('datetime')[col]
          .interpolate(method='time')
          .fillna(method='ffill')
          .fillna(method='bfill')
          .values
    )

def imputar_vizinho(df, col='temperatura'):
    return df[col].fillna(method='ffill').fillna(method='bfill')

def imputar_randomica(series, fases):
    opt_min = min(f['optMinT'] for f in fases)
    opt_max = max(f['optMaxT'] for f in fases)
    return series.apply(lambda x: random.uniform(opt_min, opt_max) if pd.isna(x) else x)

def imputar_ideal(series, fases):
    midpoint = sum((f['optMinT'] + f['optMaxT'])/2 for f in fases) / len(fases)
    return series.fillna(midpoint)

# Função de pré-processamento com logs e validações
def preprocessar_dados(input_csv, output_csv, estrategia, fases_json=None):
    print(f"Iniciando pré-processamento de: {input_csv}")
    # Assegura diretório de saída
    out_dir = os.path.dirname(output_csv)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    # Leitura robusta do CSV
    try:
        df = pd.read_csv(
            input_csv,
            sep=';', decimal=',', quotechar='"', encoding='utf-8-sig',
            dayfirst=True, low_memory=False
        )
        print("CSV lido com sucesso.")
    except Exception as e:
        sys.exit(f"Erro ao ler CSV: {e}")

    # Validação de colunas
    date_col = 'Data'
    time_col = 'Hora (UTC)'
    temp_col = 'Temp. [Hora] (C)'
    for col in (date_col, time_col, temp_col):
        if col not in df.columns:
            sys.exit(f"Erro: coluna '{col}' não encontrada.")

    # Processa datetime e temperatura
    df[time_col] = df[time_col].astype(str).str.zfill(4)
    df['datetime'] = pd.to_datetime(
        df[date_col] + ' ' + df[time_col],
        format='%d/%m/%Y %H%M', errors='coerce'
    )
    df.dropna(subset=['datetime'], inplace=True)
    df[temp_col] = pd.to_numeric(df[temp_col], errors='coerce')

    # Imputação
    if estrategia == 'interpolacao':
        df['temperatura'] = imputar_interpolacao(df, temp_col)
    elif estrategia == 'vizinho':
        df['temperatura'] = imputar_vizinho(df, temp_col)
    elif estrategia in ('randomica', 'randômica'):  # aceitando ambas strings
        fases = carregar_fases(fases_json)
        df['temperatura'] = imputar_randomica(df[temp_col], fases)
    elif estrategia == 'ideal':
        fases = carregar_fases(fases_json)
        df['temperatura'] = imputar_ideal(df[temp_col], fases)
    else:
        sys.exit(f"Estratégia inválida: {estrategia}")

    df.dropna(subset=['temperatura'], inplace=True)
    print("Imputação concluída, NaNs restantes removidos.")

    # Agregação diária
    df['date_only'] = df['datetime'].dt.date
    diario = df.groupby('date_only')['temperatura'].agg(Tmax='max', Tmin='min').reset_index()
    diario.dropna(inplace=True)
    print(f"Agregados {len(diario)} dias válidos.")

    # Salvamento
    diario['Data'] = pd.to_datetime(diario['date_only']).dt.strftime('%d/%m/%Y')
    diario[['Data', 'Tmax', 'Tmin']].to_csv(
        output_csv, index=False, sep=';', decimal='.', float_format='%.1f'
    )
    print(f"Dados salvos em {output_csv} usando '{estrategia}'")

# Interface: argv (fluxo existente) ou interativo
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Pré-processa CSV horário em diário com imputação')
    parser.add_argument('input_csv', nargs='?', help='Arquivo CSV horário')
    parser.add_argument('output_csv', nargs='?', help='Arquivo CSV diário de saída')
    parser.add_argument('-e', '--estrategia',
                        choices=['interpolacao', 'vizinho', 'randomica', 'randômica', 'ideal'],
                        help='Estratégia de imputação')
    parser.add_argument('-f', '--fases', default=DEFAULT_FASES_JSON,
                        help='JSON de fases (para randômica/ideal)')
    args = parser.parse_args()

    # Se não passou args, entra no modo interativo (fluxo manual)
    if not args.input_csv or not args.output_csv or not args.estrategia:
        print("=== Preprocessamento Interativo ===")
        sel = input("Escolha 1) interpolacao 2) vizinho 3) randomica 4) ideal: ")
        opts = {'1':'interpolacao','2':'vizinho','3':'randomica','4':'ideal'}
        est = opts.get(sel)
        if not est:
            sys.exit("Seleção inválida.")
        in_csv = args.input_csv or 'dados_horario.csv'
        out_csv = args.output_csv or 'dados_diarios.csv'
        preprocessar_dados(in_csv, out_csv, est, args.fases)
    else:
        preprocessar_dados(args.input_csv, args.output_csv, args.estrategia, args.fases)
