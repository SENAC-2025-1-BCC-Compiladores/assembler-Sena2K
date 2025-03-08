#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#define HEADERSIZE 4         
#define MEMORYSIZE 512       
#define LINESIZE 16          
#define DATA_START 0x100     

#define OPCODE_NOP  0x00
#define OPCODE_STA  0x10
#define OPCODE_LDA  0x20
#define OPCODE_ADD  0x30
#define OPCODE_OR   0x40
#define OPCODE_AND  0x50
#define OPCODE_NOT  0x60
#define OPCODE_JMP  0x80
#define OPCODE_JMN  0x90
#define OPCODE_JMZ  0xA0
#define OPCODE_HLT  0xF0

#define RESULTOFFSET (DATA_START + 4)

// tabela de símbolos pra seção DATA (nome, endereço, valor, se foi definido)
typedef struct {
    char name[32];
    int address; // endereço na memoria
    int value;   // valor pra DB
    bool defined;
} Symbol;

#define MAX_SYMBOLS 32
Symbol symbols[MAX_SYMBOLS];
int symbolCount = 0;

// adiciona um simbolo na tabela 
void addSymbol(const char* name, int address, int value, bool defined) {
    if (symbolCount < MAX_SYMBOLS) {
        strncpy(symbols[symbolCount].name, name, sizeof(symbols[symbolCount].name));
        symbols[symbolCount].address = address;
        symbols[symbolCount].value = value;
        symbols[symbolCount].defined = defined;
        symbolCount++;
    }
}

// procura um simbolo pelo nome. Retorna o endereço se achar
int findSymbol(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbols[i].name, name) == 0)
            return symbols[i].address;
    }
    return -1; 
}

// converte string pra numero
int parseNumber(const char* str) {
    if (str[0]=='0' && (str[1]=='x' || str[1]=='X'))
        return (int)strtol(str, NULL, 16);
    else
        return atoi(str);
}

bool assemble(const char* inputFile, const char* outputFile) {
    FILE *fin = fopen(inputFile, "r");
    if (!fin) {
        perror("Erro ao abrir o arquivo assembly");
        return false;
    }
    
    uint8_t memory[MEMORYSIZE] = {0};
    
    // header pro neander
    uint8_t header[HEADERSIZE] = {0x03, 0x4E, 0x44, 0x52};
    memcpy(memory, header, HEADERSIZE);
    
    int dataAddr = DATA_START;
    
    // ORG define onde o codigo começa 
    // endereco real = HEADERSIZE + org * 2 
    int codeOrigin = 0;
    int codeStart = HEADERSIZE + codeOrigin * 2; 
    int codeAddr = codeStart; 
    
    enum { NONE, DATA_SECTION, CODE_SECTION } section = NONE;
    
    char line[256];
    while (fgets(line, sizeof(line), fin)) {
        line[strcspn(line, "\r\n")] = '\0';
        // pula linhas vazias ou comentarios 
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == ';') continue;
        
        if (strcasecmp(p, ".DATA") == 0) {
            section = DATA_SECTION;
            continue;
        }
        if (strcasecmp(p, ".CODE") == 0) {
            section = CODE_SECTION;
            continue;
        }
        
        if (section == DATA_SECTION) {
            char label[32], directive[16], valueStr[32];
            if (sscanf(p, "%31s %15s %31s", label, directive, valueStr) < 3)
                continue;
            if (strcasecmp(directive, "DB") == 0) {
                int value = 0;
                bool defined = true;
                if (strcmp(valueStr, "?") == 0) {
                    defined = false;
                    value = 0; 
                } else {
                    value = parseNumber(valueStr);
                }
                if (dataAddr % 2 != 0) dataAddr++;
                addSymbol(label, dataAddr, value, defined);
                memory[dataAddr] = (uint8_t)value;
                memory[dataAddr+1] = 0;
                dataAddr += 2;
            }
        } else if (section == CODE_SECTION) {
            if (strncasecmp(p, ".ORG", 4) == 0) {
                int org;
                if (sscanf(p, ".ORG %d", &org) == 1) {
                    codeOrigin = org;
                    codeStart = HEADERSIZE + codeOrigin * 2;
                    codeAddr = codeStart;
                }
                continue;
            }
            char mnemonic[16], operand[32];
            int items = sscanf(p, "%15s %31s", mnemonic, operand);
            uint8_t opcode = 0;
            uint8_t operandByte = 0;
            if (strcasecmp(mnemonic, "LDA") == 0) {
                opcode = OPCODE_LDA;
            } else if (strcasecmp(mnemonic, "ADD") == 0) {
                opcode = OPCODE_ADD;
            } else if (strcasecmp(mnemonic, "STA") == 0) {
                opcode = OPCODE_STA;
            } else if (strcasecmp(mnemonic, "HLT") == 0) {
                opcode = OPCODE_HLT;
            } else if (strcasecmp(mnemonic, "NOP") == 0) {
                opcode = OPCODE_NOP;
            } else {
                continue;
            }
            
            if (opcode != OPCODE_HLT && items == 2) {
                int symAddr = findSymbol(operand);
                if (symAddr < 0) {
                    fprintf(stderr, "Símbolo indefinido: %s\n", operand);
                    fclose(fin);
                    return false;
                }
                operandByte = (uint8_t)((symAddr - HEADERSIZE) / 2);
            }
            
            memory[codeAddr]   = opcode;
            memory[codeAddr+1] = 0;
            memory[codeAddr+2] = operandByte;
            memory[codeAddr+3] = 0;
            codeAddr += 4;
        }
    }
    fclose(fin);
    
    FILE *fout = fopen(outputFile, "wb");
    if (!fout) {
        perror("Erro ao criar o arquivo de memória");
        return false;
    }
    fwrite(memory, 1, MEMORYSIZE, fout);
    fclose(fout);
    
    printf("Montagem concluída. Arquivo '%s' criado.\n", outputFile);
    return true;
}

void print_memory(uint8_t *bytes, size_t size) {
    size_t offset = 0;
    while (offset < size) {
        printf("%08zx: ", offset);
        for (size_t i = 0; i < LINESIZE; i++) {
            if (offset + i < size)
                printf("%02x ", bytes[offset + i]);
            else
                printf("   ");
        }
        printf("\n");
        offset += LINESIZE;
    }
}
    
int main(void) {
    if (!assemble("teste.txt", "teste.mem")) {
        fprintf(stderr, "Falha na montagem.\n");
        return 1;
    }
    
    return 0;
}
