cmake_minimum_required(VERSION 3.27)

#==============================================================================
# MC859 - Projeto em Teoria da Computação
#
# Autor: Guilherme G. Brandt (RA: 235970)
# Problema: Strip Packing em Níveis com Pesos
#==============================================================================

project(mc859 CXX)

include(ExternalProject)

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

#==============================================================================
# Configuração dos pacotes externos
#------------------------------------------------------------------------------
# Nesta seção configuramos as bibliotecas externas usadas no projeto.
#==============================================================================

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# LEMON - Biblioteca de código aberto para otimização combinatória em redes.
#
# Configure a variável LEMON_DIR no CMAKE ou a variável de ambiente LEMON_HOME
# para definir a pasta de instalação da biblioteca.
#
# Veja o arquivo cmake/FindLEMON.cmake para mais detalhes de como a biblioteca
# é configurada.
find_package(LEMON) # TODO: marcar como REQUIRED quando necessário

# Gurobi - Solver de otimização matemática.
#
# Configure a variável GUROBI_DIR no CMAKE ou a variável de ambiente
# GUROBI_HOME (esta segunda costuma ser configurada por padrão com a instalação
# da biblioteca) para definir a pasta de instalação da biblioteca.
#
# Veja o arquivo cmake/FindGUROBI.cmake para mais detalhes de como a biblioteca
# é configurada.
find_package(GUROBI) # TODO: marcar como REQUIRED quando necessário

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

#==============================================================================
# Configuração de compilação
#==============================================================================

include_directories(
  ${LEMON_INCLUDE_DIR}
  ${GUROBI_INCLUDE_DIRS}
  ${BRKGA_INCLUDE_DIR}
  include)

find_package(Threads)
find_package(OpenMP)

add_executable(mc859-strip-packing src/main.cpp)

target_compile_options(mc859-strip-packing PRIVATE -Wall -Wextra -Wpedantic)

if(Threads_FOUND)
  target_link_libraries(mc859-strip-packing PUBLIC Threads::Threads)
endif()

if(OpenMP_CXX_FOUND)
  target_link_libraries(mc859-strip-packing PUBLIC OpenMP::OpenMP_CXX)
endif()
