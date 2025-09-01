#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cnfparser.h"
#include "solver.h"

// 保存结果到文件
void saveResult(const char* filename, int result, int* assignment, int varCount, long long time_ms) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Cannot create result file: %s\n", filename);
        return;
    }
    
    // 写入结果状态 (1=SAT, 0=UNSAT, -1=UNKNOWN)
    fprintf(file, "s %d\n", result);
    
    // 如果满足，写入变量赋值
    if (result == 1 && assignment) {
        fprintf(file, "v ");
        for (int i = 1; i <= varCount; i++) {
            // 只输出已赋值的变量
            if (assignment[i] != 0) {
                if (assignment[i] == 1) {
                    fprintf(file, "%d ", i);
                } else {
                    fprintf(file, "-%d ", i);
                }
            }
        }
        fprintf(file, "0\n");  // DIMACS格式要求以0结尾
    }
    
    // 写入执行时间
    fprintf(file, "t %lld\n", time_ms);
    
    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <cnf_file>\n", argv[0]);
        return 1;
    }
    
    char* input_file = argv[1];
    
    // 解析CNF文件
    printf("Parsing CNF file: %s\n", input_file);
    int parse_result = parseCNFFile(input_file);
    if (parse_result != 0) {
        printf("Error parsing CNF file\n");
        return 1;
    }
    
    // 记录开始时间
    clock_t start_time = clock();
    
    // 调用DPLL求解器
    printf("Solving SAT problem...\n");
    printf("Variables: %d, Clauses: %d\n", numVariables, numClauses);
    int result = solveSAT();
    
    // 计算执行时间
    clock_t end_time = clock();
    long long execution_time = ((long long)(end_time - start_time) * 1000) / CLOCKS_PER_SEC;
    
    // 输出求解结果
    printf("Result: %s\n", result == 1 ? "SAT" : result == 0 ? "UNSAT" : "UNKNOWN");
    printf("Time: %lld ms\n", execution_time);
    
    // 保存结果到res文件夹
    char output_file[256];
    char base_filename[256];
    
    // 提取文件名（不包含路径）
    char* filename_only = strrchr(input_file, '/');
    if (!filename_only) filename_only = strrchr(input_file, '\\');
    if (filename_only) {
        filename_only++; // 跳过路径分隔符
    } else {
        filename_only = input_file; // 没有路径，直接使用文件名
    }
    
    strcpy(base_filename, filename_only);
    char* dot = strrchr(base_filename, '.');
    if (dot) *dot = '\0';
    
    // 创建res文件夹下的路径
    sprintf(output_file, "res/%s.res", base_filename);
    
    saveResult(output_file, result, variableAssignment, numVariables, execution_time);
    printf("Result saved to: %s\n", output_file);
    
    // 清理内存
    freeCNFData();
    if (variableAssignment) {
        free(variableAssignment);
        variableAssignment = NULL;
    }
    
    return 0;
}
