# MC859 - Projeto

O projeto está organizado da seguinte forma:
- `CMakeLists.txt` - Arquivo com instruções de compilação para o CMake.
- `Install-Lemon-in-subfolder.bash` - Script para instalação da biblioteca LEMON.
- `include/` - Pasta com os arquivos de cabeçalho.
  - `mylib/` - Cabeçalhos da biblioteca auxiliar.
  - `strip_packing/` - Cabeçalhos dos algoritmos relacionados ao problema.
    - `defs.hpp` - Definição das estruturas de instância e solução do problema.
    - `heuristics.hpp` - Definição das heurísticas.
    - `exact.hpp` - Algoritmo exato para problema.
    - `flow.hpp` - Algoritmo exato para problema (formulação por fluxo).
- `src/` - Pasta com os arquivos de código C++.
  - `mylib/` - Código fonte da biblioteca auxiliar.
  - `exact.cpp` - Arquivo principal do programa da formulação direta.
  - `flow.cpp` - Arquivo principal do programa da formulação por fluxo.
  - `heuristics.cpp` - Arquivo principal do programa das heurísticas.
- `instances/` - Instâncias do problema utilizadas para teste.
- `report/` - Relatórios.
  - `exact/main.pdf` - Relatório da parte 2.

O projeto pode ser compilado com uma versão compatível do CMake (3.22+) e um compilador com suporte a C++ 20. Testado com CMake 3.28 e GCC 13.2.1.

Para compilar:
```
mkdir -p build && cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
```

Para executar os programas, use:
```
./mc859-strip-packing <instancia>
./mc859-strip-packing-flow <instancia>
```

Onde `<instancia>` corresponde ao caminho de um arquivo de instância do problema.
