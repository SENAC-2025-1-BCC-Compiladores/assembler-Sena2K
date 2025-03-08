[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/FLG6_3H5)
Murilo dos Santos Sena

# Montador Simples para Neander

Este é um montador simples em C para um processador inspirado na arquitetura Neander. Ele lê um arquivo assembly (por exemplo, `teste.txt`) e gera uma imagem de memória (`teste.mem`) com 512 bytes, contendo tanto a seção de dados quanto o código.

## Funcionalidades

- **Cabeçalho e Memória:**  
  - Cabeçalho fixo de 4 bytes (definido para o Neander).  
  - Memória total de 512 bytes.

- **Seção DATA:**  
  - Os dados são armazenados a partir do endereço `DATA_START` (0x100).  
  - Cada entrada de dados é definida com a diretiva `DB` e armazenada em 2 bytes para garantir alinhamento (cada palavra tem 2 bytes).

- **Seção CODE:**  
  - O código é montado a partir de um endereço calculado com a diretiva `.ORG`.  
  - O endereço real do código é:  
    ```
    endereço real = HEADERSIZE + (.ORG * 2)
    ```
    ou seja, se `.ORG` for zero, o código começa logo após o cabeçalho.
  - Cada instrução ocupa 4 bytes.

- **Tabela de Símbolos:**  
  - Gerencia os rótulos (labels) definidos na seção DATA, permitindo que sejam referenciados nas instruções do código.

- **OpCodes:**  
  - Instruções como `LDA`, `ADD`, `STA`, `HLT`, etc., são definidas com seus respectivos códigos (por exemplo, `OPCODE_LDA` é 0x20).

## Estrutura do Projeto

- `main.c`: Código fonte do montador.
- `Makefile`: Arquivo para compilação usando o comando `make`.
- `teste.txt`: Arquivo assembly de exemplo (deve ser criado conforme suas necessidades).
- `teste.mem`: Arquivo de saída gerado após a montagem.

## Compilação

Para compilar basta executar:

```bash
make

```bash
make run

Isso compila o código e gera o executável montador.

