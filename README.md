# Gerenciamento de Memória Virtual

Projeto desenvolvido em linguagem C para simular o funcionamento de um sistema de memória virtual, utilizando paginação, TLB (Translation Lookaside Buffer) e algoritmos de substituição de páginas.

## Funcionalidades

- Tradução de endereços lógicos para endereços físicos.
- Gerenciamento de tabela de páginas.
- Utilização de TLB com 16 entradas.
- Consulta paralela da TLB utilizando threads POSIX (pthread).
- Tratamento de Page Faults.
- Substituição de páginas utilizando FIFO ou LRU.
- Substituição de entradas da TLB utilizando FIFO ou LRU.
- Geração do arquivo `correct.txt` com os resultados das traduções.
- Cálculo de estatísticas de acesso à memória.

## Configurações

| Parâmetro | Valor |
|---|---:|
| Tamanho da página | 256 bytes |
| Número de páginas | 256 |
| Número de frames | 128 |
| Tamanho da TLB | 16 entradas |
| Tamanho da memória física | 32.768 bytes |

## Requisitos

- GCC
- GNU Make
- Linux ou ambiente compatível com pthreads POSIX.

## Compilação

Para compilar o projeto, execute:

```bash
make
```

Para remover o executável gerado:

```bash
make clean
```

## Execução

O programa recebe três argumentos:

1. Arquivo contendo os endereços lógicos.
2. Algoritmo de substituição de páginas (`fifo` ou `lru`).
3. Algoritmo de substituição da TLB (`fifo` ou `lru`).

Sintaxe:

```bash
./vm <arquivo_de_enderecos> <algoritmo_paginas> <algoritmo_tlb>
```

Exemplo:

```bash
./vm addresses.txt lru fifo
```

Nesse exemplo, o algoritmo LRU será utilizado para substituição de páginas e FIFO para substituição das entradas da TLB.

## Arquivos necessários

O programa utiliza os seguintes arquivos:

- `BACKING_STORE.bin`: arquivo que contém os dados da memória secundária.
- `addresses.txt`: arquivo contendo os endereços lógicos que serão traduzidos.

Esses arquivos devem estar disponíveis no diretório de execução do programa.

## Arquivo de saída

Após a execução, o programa gera o arquivo:

```text
correct.txt
```

Esse arquivo contém os endereços virtuais processados, seus respectivos endereços físicos e os valores encontrados na memória.

O programa também apresenta no terminal estatísticas da execução:

- Número de endereços traduzidos.
- Quantidade de Page Faults.
- Taxa de Page Faults.
- Quantidade de TLB Hits.
- Quantidade de TLB Misses.
- Taxa de TLB Hits.

## Algoritmos de substituição

O projeto implementa dois algoritmos de substituição:

**FIFO (First-In, First-Out)**

Substitui a página ou entrada da TLB que está há mais tempo no sistema.

**LRU (Least Recently Used)**

Substitui a página ou entrada da TLB que permaneceu por mais tempo sem ser utilizada.

## Autores

Projeto acadêmico desenvolvido como atividade da disciplina de Sistemas Operacionais.