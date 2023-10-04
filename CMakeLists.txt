cmake_minimum_required(VERSION 3.27)

#==============================================================================
# MC859 - Projeto em Teoria da Computação
#
# Autor: Guilherme G. Brandt (RA: 235970)
# Problema: Strip Packing em Níveis com Pesos
#==============================================================================

project(mc859 CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

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

#==============================================================================
# Configuração de compilação
#==============================================================================

include_directories(${LEMON_INCLUDE_DIR} ${GUROBI_INCLUDE_DIRS} include)

add_executable(mc859-strip-packing src/main.cpp)
