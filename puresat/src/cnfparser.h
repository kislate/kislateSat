#ifndef CNFPARSER_H
#define CNFPARSER_H

// 全局变量声明（用于存储解析后的数据）
extern int numVariables;
extern int numClauses;
extern int** clauses;
extern int* clauseSizes;

// 函数声明
int parseCNFFile(const char* filename);
void printClauses();
void freeCNFData();

#endif
