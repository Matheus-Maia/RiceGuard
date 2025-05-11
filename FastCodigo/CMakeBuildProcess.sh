#!/bin/bash

# Cria o diretório de build se ele não existir
mkdir -p build

# Navega para o diretório de build
cd build

# Executa o CMake com o tipo de build configurado para Debug
# O '..' indica que o CMakeLists.txt está no diretório pai
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Compila o projeto
# O make usará as instruções geradas pelo CMake
make

# Executa os testes com output verboso
# ctest é a ferramenta de teste do CMake
ctest --verbose

