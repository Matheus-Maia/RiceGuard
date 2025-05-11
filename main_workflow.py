import subprocess
import os
import sys
import time
import platform

# Função para executar e validar comandos
def run_command(command, step_name):
    print(f"\n--- Iniciando Etapa: {step_name} ---")
    print(f"Executando: {' '.join(command)}")
    start_time = time.time()
    try:
        process = subprocess.run(command, check=True, capture_output=True, text=True, encoding='utf-8')
        print(f"Saída Padrão:\n{process.stdout}")
        if process.stderr:
            print(f"Saída de Erro Padrão:\n{process.stderr}")
        end_time = time.time()
        print(f"--- Etapa '{step_name}' concluída em {end_time - start_time:.2f}s ---")
        return True
    except Exception as e:
        print(f"Erro na etapa '{step_name}': {e}")
        return False

# Prompt interativo para escolha de estratégia
def escolher_estrategia():
    print("Escolha a estratégia de imputação para o pré-processamento:")
    print(" 1) interpolacao")
    print(" 2) vizinho")
    print(" 3) randomica")
    print(" 4) ideal")
    escolha = input("Digite (1-4): ")
    mapping = {'1': 'interpolacao', '2': 'vizinho', '3': 'randomica', '4': 'ideal'}
    estrategia = mapping.get(escolha)
    if not estrategia:
        print("Opção inválida. Encerrando.")
        sys.exit(1)
    return estrategia

if __name__ == "__main__":
    # Configuração de caminhos
    project_root = os.path.dirname(os.path.abspath(__file__))
    input_csv_original = os.path.join(project_root, 'dados_ano.csv')
    preprocessed_dir = os.path.join(project_root, 'preprocessed')
    preprocessed_csv = os.path.join(preprocessed_dir, 'dados_diarios.csv')
    preprocess_script = os.path.join(project_root, 'helpers', 'preprocessamento.py')

    core_logic_dir = os.path.join(project_root, 'FastCodigo', 'build')
    executable_name = 'analise.exe' if platform.system() == 'Windows' else 'analise'
    cpp_executable = os.path.join(core_logic_dir, executable_name)

    processed_dir = os.path.join(project_root, 'processados')
    resumo_csv_cpp = os.path.join(processed_dir, 'resumo_mensal.csv')
    detalhe_csv_cpp = os.path.join(processed_dir, 'analise_detalhada.csv')

    postprocess_script = os.path.join(project_root, 'helpers', 'posprocessamento.py')
    reports_dir = os.path.join(project_root, 'relatorios')

    # Verificações iniciais
    for path, desc in [(input_csv_original, 'dados original'), (preprocess_script, 'script de pré-processamento'),
                       (cpp_executable, 'executável C++'), (postprocess_script, 'script de pós-processamento')]:
        if not os.path.exists(path):
            print(f"Erro crítico: {desc} não encontrado em {path}")
            sys.exit(1)

    # Etapa 1: Pré-processamento interativo
    os.makedirs(preprocessed_dir, exist_ok=True)
    estrategia = escolher_estrategia()
    command_preprocess = [sys.executable, preprocess_script,
                          input_csv_original, preprocessed_csv,
                          '--estrategia', estrategia, '--fases',
                          os.path.join(project_root, 'helpers', 'fases_cultivo_arroz.json')]
    if not run_command(command_preprocess, "Pré-processamento (Python)"):
        sys.exit(1)

    # Etapa 2: Análise Principal (C++)
    os.makedirs(processed_dir, exist_ok=True)
    command_cpp = [cpp_executable, preprocessed_csv, processed_dir]
    if not run_command(command_cpp, "Análise Principal (C++)"):
        sys.exit(1)

    # Etapa 3: Pós-processamento (Gráficos)
    os.makedirs(reports_dir, exist_ok=True)
    command_post = [sys.executable, postprocess_script,
                    resumo_csv_cpp, detalhe_csv_cpp, reports_dir]
    if not run_command(command_post, "Pós-processamento e Relatórios"):
        sys.exit(1)

    print("\nWorkflow completo executado com sucesso!")
    print(f"Dados pré-processados: {preprocessed_dir}")
    print(f"Resultados C++: {processed_dir}")
    print(f"Relatórios: {reports_dir}")
