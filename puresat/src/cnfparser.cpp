#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnfparser.h"

// 全局变量定义
int numVariables = 0;
int numClauses = 0;
int** clauses = NULL;
int* clauseSizes = NULL;

int parseCNFFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open file: %s\n", filename);
        return -1;
    }
    
    char line[1024];
    int clauseIndex = 0;
    
    // 第一遍扫描：计算子句数量
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue;  // 跳过注释行
        if (line[0] == 'p') {
            // 解析问题行
            sscanf(line, "p cnf %d %d", &numVariables, &numClauses);
            break;
        }
    }
    
    // 分配内存
    clauses = (int**)malloc(numClauses * sizeof(int*));
    clauseSizes = (int*)malloc(numClauses * sizeof(int));
    if (!clauses || !clauseSizes) {
        printf("Memory allocation failed\n");
        fclose(file);
        return -1;
    }
    
    // 重新读取文件，解析子句
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') continue;
        
        if (clauseIndex < numClauses) {
            // 计算这一行有多少个文字
            int temp_literals[1024];
            int literal_count = 0;
            char* token = strtok(line, " \t\n");
            
            while (token && atoi(token) != 0) {
                temp_literals[literal_count++] = atoi(token);
                token = strtok(NULL, " \t\n");
            }
            
            // 分配内存存储这个子句
            clauses[clauseIndex] = (int*)malloc((literal_count + 1) * sizeof(int));
            clauseSizes[clauseIndex] = literal_count;
            
            for (int i = 0; i < literal_count; i++) {
                clauses[clauseIndex][i] = temp_literals[i];
            }
            clauses[clauseIndex][literal_count] = 0;  // 结束标记
            
            clauseIndex++;
        }
    }
    
    fclose(file);
    return 0;
}

void printClauses() {
    printf("Variables: %d, Clauses: %d\n", numVariables, numClauses);
    for (int i = 0; i < numClauses; i++) {
        printf("Clause %d: ", i);
        for (int j = 0; j < clauseSizes[i]; j++) {
            printf("%d ", clauses[i][j]);
        }
        printf("\n");
    }
}

void freeCNFData() {
    if (clauses) {
        for (int i = 0; i < numClauses; i++) {
            if (clauses[i]) {
                free(clauses[i]);
            }
        }
        free(clauses);
        clauses = NULL;
    }
    if (clauseSizes) {
        free(clauseSizes);
        clauseSizes = NULL;
    }
    numVariables = 0;
    numClauses = 0;
}
