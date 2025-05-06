cmake_minimum_required(VERSION 3.15)
project(FastCodigo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-O3 -march=native -ffast-math -funroll-loops)
include_directories(${PROJECT_SOURCE_DIR}/include)
find_package(OpenMP REQUIRED)
find_package(GTest REQUIRED)

add_library(core_lib src/core_logic/analise_viabilidade.cpp)
target_link_libraries(core_lib PUBLIC OpenMP::OpenMP_CXX)

add_library(io_lib src/io/csv_reader.cpp)
add_library(summary_lib src/summary/summary_generator.cpp)

add_executable(analise src/main.cpp)
target_link_libraries(analise PRIVATE core_lib io_lib summary_lib)

enable_testing()
# Testes
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.cpp")
add_executable(test_analise ${TEST_SOURCES})
target_link_libraries(test_analise PRIVATE core_lib io_lib summary_lib GTest::GTest GTest::Main OpenMP::OpenMP_CXX)
add_test(NAME AnaliseTests COMMAND test_analise)