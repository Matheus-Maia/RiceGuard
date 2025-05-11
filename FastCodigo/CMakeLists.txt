cmake_minimum_required(VERSION 3.15)
project(FastCodigo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# Opções de compilação
add_compile_options(
        -O3
        -march=native
        -ffast-math
        -funroll-loops
        -Wall
        -Wextra
        -Wpedantic
)

include_directories(${PROJECT_SOURCE_DIR}/src)
find_package(OpenMP REQUIRED)
find_package(GTest REQUIRED)

# Fontes das bibliotecas
set(VIAB_SOURCES
        src/analise/analise_viabilidade.cpp
        src/model/viab/fase.cpp
)

set(IO_SOURCES
        src/model/io/csv_reader.cpp
        src/model/io/json_loader.cpp
)

set(SUMMARY_SOURCES
        src/model/summary/summary_generator.cpp
)

# Criar bibliotecas
add_library(viab_lib ${VIAB_SOURCES})
target_link_libraries(viab_lib PUBLIC OpenMP::OpenMP_CXX)

add_library(io_lib ${IO_SOURCES})
target_link_libraries(io_lib PUBLIC viab_lib) # Importante: io_lib depende de viab_lib

add_library(summary_lib ${SUMMARY_SOURCES})
target_link_libraries(summary_lib PUBLIC viab_lib) # summary_lib também usa tipos do viab_lib

# Executável principal
add_executable(analise src/main.cpp)
target_link_libraries(analise PRIVATE viab_lib io_lib summary_lib)

# Testes
enable_testing()
add_executable(test_analise src/tests/teste_analise_viabilidade.cpp)
target_link_libraries(test_analise PRIVATE viab_lib io_lib summary_lib GTest::GTest GTest::Main)
add_test(NAME AnaliseTests COMMAND test_analise)