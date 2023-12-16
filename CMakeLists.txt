cmake_minimum_required(VERSION 3.27)

#==============================================================================
# MC859 - Projeto em Teoria da Computação
#
# Autor: Guilherme G. Brandt (RA: 235970)
# Problema: Strip Packing em Níveis com Pesos
#==============================================================================

project(mc859 CXX)

include(FetchContent)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.4)
        message(FATAL_ERROR "GCC version must be at least 9.4!")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "Clang version must be at least 10.0!")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#==============================================================================
# Configuração dos pacotes externos
#------------------------------------------------------------------------------
# Nesta seção configuramos as bibliotecas externas usadas no projeto.
#==============================================================================

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Gurobi - Solver de otimização matemática.
#
# Configure a variável GUROBI_DIR no CMAKE ou a variável de ambiente
# GUROBI_HOME (esta segunda costuma ser configurada por padrão com a instalação
# da biblioteca) para definir a pasta de instalação da biblioteca.
#
# Veja o arquivo cmake/FindGUROBI.cmake para mais detalhes de como a biblioteca
# é configurada.
find_package(GUROBI REQUIRED)

# BRKGA-MP-IPR - Biblioteca de Multi-Parent Biased Random-Key Genetic Algorithm
# com Implicit Path Relinking.
#
# Repositório: https://github.com/ceandrade/brkga_mp_ipr_cpp
#
# Faça download do submodulo do git, ou configure a variável de ambiente
# BRKGA_HOME com o caminho para o repositório clonado.
#
# Bibliografia:
#   C.E. Andrade. R.F. Toso, J.F. Gonçalves, M.G.C. Resende. The Multi-Parent
#   Biased Random-key Genetic Algorithm with Implicit Path Relinking. European
#   Journal of Operational Research, volume 289, number 1, pages 17–30, 2021.
#   DOI 10.1016/j.ejor.2019.11.037
find_package(BRKGA REQUIRED)

# yaml-cpp - Biblioteca para leitura de arquivos YAML.
#
# Nenhuma configuração adicional é necessária.
FetchContent_Declare(YAML_CPP
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG        0.8.0
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(YAML_CPP)

# Blend2d - Biblioteca para renderização de gráficos vetoriais.
#
# Nenhuma configuração adicional é necessária.
FetchContent_Declare(BLEND2D
  GIT_REPOSITORY git@github.com:GuiBrandt/blend2d.git
  GIT_TAG        3cef72dc
  GIT_SHALLOW    OFF
)

set(BLEND2D_NO_JIT ON)
set(BLEND2D_STATIC ON)

FetchContent_MakeAvailable(BLEND2D)

# argparse - Biblioteca para processamento de argumentos de linha de comando.
#
# Nenhuma configuração adicional é necessária.
FetchContent_Declare(ARGPARSE
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG        v2.9
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(ARGPARSE)

include_directories(
  ${GUROBI_INCLUDE_DIRS}
  ${BRKGA_INCLUDE_DIR}
  include)

find_package(Threads)
find_package(OpenMP)

#==============================================================================
# Configuração de compilação
#==============================================================================
#------------------------------------------------------------------------------
# Exato
#------------------------------------------------------------------------------
add_executable(mc859-strip-packing src/exact.cpp)

target_link_libraries(mc859-strip-packing PRIVATE
  yaml-cpp::yaml-cpp
  argparse::argparse
  blend2d::blend2d
  ${GUROBI_CXX_LIBRARY}
  ${GUROBI_C_LIBRARY})

#------------------------------------------------------------------------------
# Heurísticas
#------------------------------------------------------------------------------
add_executable(mc859-strip-packing-heuristics src/heuristics.cpp)

target_compile_options(mc859-strip-packing-heuristics PRIVATE
  -Wall -Wextra -Wpedantic)

target_link_libraries(mc859-strip-packing-heuristics PRIVATE
  yaml-cpp::yaml-cpp
  argparse::argparse
  blend2d::blend2d)

# Habilita threads e OpenMP se possível (usado no BRKGA).
if(Threads_FOUND)
  target_link_libraries(mc859-strip-packing-heuristics PRIVATE Threads::Threads)
endif()
if(OpenMP_CXX_FOUND)
  target_link_libraries(mc859-strip-packing-heuristics PRIVATE OpenMP::OpenMP_CXX)
endif()

#------------------------------------------------------------------------------
# Gerador de instâncias
#------------------------------------------------------------------------------
add_executable(mc859-strip-packing-gen-instances src/gen_instances.cpp)

target_compile_options(mc859-strip-packing-gen-instances PRIVATE
  -Wall -Wextra -Wpedantic)

target_link_libraries(mc859-strip-packing-gen-instances PRIVATE
  yaml-cpp::yaml-cpp
  argparse::argparse)
