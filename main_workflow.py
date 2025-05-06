# main_workflow.py
import subprocess
import os
import sys
import time
import platform

def run_command(command, step_name):
    """Executa um comando no shell e verifica erros."""
    print(f"\n--- Iniciando Etapa: {step_name} ---")
    print(f"Executando: {' '.join(command)}")
    start_time = time.time()
    try:
        # Usar shell=True pode ser um risco de segurança se os caminhos vierem de fontes não confiáveis
        # Mas é mais simples para lidar com caminhos e comandos complexos.
        # Alternativa: passar a lista 'command' diretamente sem shell=True se não precisar de recursos do shell.
        process = subprocess.run(command, check=True, capture_output=True, text=True, encoding='utf-8')
        print(f"Saída Padrão:\n{process.stdout}")
        if process.stderr:
             print(f"Saída de Erro Padrão:\n{process.stderr}") # Mostrar avisos do C++
        end_time = time.time()
        print(f"--- Etapa '{step_name}' concluída com sucesso em {end_time - start_time:.2f} segundos ---")
        return True
    except FileNotFoundError:
        print(f"Erro: Comando não encontrado: {command[0]}. Verifique se está no PATH ou se o caminho está correto.")
        return False
    except subprocess.CalledProcessError as e:
        print(f"Erro ao executar a etapa '{step_name}'. Comando retornou código {e.returncode}.")
        print(f"Comando: {' '.join(e.cmd)}")
        print(f"Saída Padrão:\n{e.stdout}")
        print(f"Saída de Erro Padrão:\n{e.stderr}")
        return False
    except Exception as e:
        print(f"Erro inesperado durante a execução da etapa '{step_name}': {e}")
        return False

if __name__ == "__main__":
    # --- Configuração de Caminhos ---
    project_root = os.path.dirname(os.path.abspath(__file__)) # Diretório onde este script está

    # Arquivo de entrada original
    input_csv_original = os.path.join(project_root, 'dados_ano.csv')

    # Saída do pré-processamento / Entrada para C++
    preprocessed_dir = os.path.join(project_root, 'preprocessed')
    preprocessed_csv = os.path.join(preprocessed_dir, 'dados_diarios.csv')

    # Script de pré-processamento
    preprocess_script = os.path.join(project_root, 'helpers', 'preprocessamento.py')

    # Executável C++ e diretório
    core_logic_dir = os.path.join(project_root, 'FastCodigo/build')
    # Determina o nome do executável baseado no SO
    executable_name = 'analise.exe' if platform.system() == "Windows" else 'analise'
    cpp_executable = os.path.join(core_logic_dir, executable_name)

    # Diretório de saída do C++ / Entrada para pós-processamento
    processed_dir = os.path.join(project_root, 'processados')
    resumo_csv_cpp = os.path.join(processed_dir, 'resumo_mensal_cpp.csv')
    detalhe_csv_cpp = os.path.join(processed_dir, 'analise_detalhada.csv')

    # Script de pós-processamento
    postprocess_script = os.path.join(project_root, 'helpers', 'posprocessamento.py')

    # Diretório de saída dos relatórios (gráficos)
    reports_dir = os.path.join(project_root, 'relatorios')
    # --- Fim Configuração ---

    # --- Verificações Iniciais ---
    if not os.path.exists(input_csv_original):
        print(f"Erro Crítico: Arquivo de dados original não encontrado: {input_csv_original}")
        sys.exit(1)
    if not os.path.exists(preprocess_script):
        print(f"Erro Crítico: Script de pré-processamento não encontrado: {preprocess_script}")
        sys.exit(1)
    if not os.path.exists(cpp_executable):
        print(f"Erro Crítico: Executável C++ não encontrado: {cpp_executable}")
        print("Certifique-se de que o código C++ foi compilado corretamente (veja instruções).")
        sys.exit(1)
    if not os.path.exists(postprocess_script):
         print(f"Erro Crítico: Script de pós-processamento não encontrado: {postprocess_script}")
         sys.exit(1)
    # --- Fim Verificações ---


    # --- Etapa 1: Pré-processamento ---
    command_preprocess = [sys.executable, preprocess_script, input_csv_original, preprocessed_csv]
    if not run_command(command_preprocess, "Pré-processamento (Python)"):
        sys.exit(1)

    # --- Etapa 2: Análise Principal (C++) ---
    # Garante que o diretório de saída do C++ exista antes de chamá-lo
    os.makedirs(processed_dir, exist_ok=True)
    command_cpp = [cpp_executable, preprocessed_csv, processed_dir]
    if not run_command(command_cpp, "Análise Principal (C++)"):
        sys.exit(1)

    # --- Etapa 3: Pós-processamento (Gráficos) ---
    # Garante que o diretório de relatórios exista
    os.makedirs(reports_dir, exist_ok=True)
    command_postprocess = [sys.executable, postprocess_script, resumo_csv_cpp, detalhe_csv_cpp, reports_dir]
    if not run_command(command_postprocess, "Pós-processamento e Relatórios (Python)"):
        sys.exit(1)

    print("\n=========================================")
    print("Workflow completo executado com sucesso!")
    print(f"Dados pré-processados em: {preprocessed_dir}")
    print(f"Resultados da análise em: {processed_dir}")
    print(f"Relatórios (gráficos) em: {reports_dir}")
    print("=========================================")