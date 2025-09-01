#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "solver.h"
#include "cnfparser.h"

int* variableAssignment = NULL;  // 全局变量赋值数组

// 检查子句是否被满足
int isClauseSatisfied(int* clause, int clauseSize, int* assignment) {
    for (int i = 0; i < clauseSize; i++) {
        int literal = clause[i];
        int var = abs(literal);
        
        if (var <= 0 || var > numVariables) continue;  // 防止数组越界
        
        int value = assignment[var];
        if (value == 0) continue;  // 未赋值
        
        if ((literal > 0 && value == 1) || (literal < 0 && value == -1)) {
            return 1;  // 子句满足
        }
    }
    return 0;  // 子句未满足
}

// 检查公式是否全部满足
int isFormulaSatisfied(int** clauses, int* clauseSizes, int numClauses, int* assignment) {
    for (int i = 0; i < numClauses; i++) {
        if (!isClauseSatisfied(clauses[i], clauseSizes[i], assignment)) {
            return 0;  // 有子句未满足
        }
    }
    return 1;  // 所有子句都满足
}

// 找到单元子句
int findUnitClause(int** clauses, int* clauseSizes, int numClauses, int* assignment, int* unitLiteral) {
    for (int i = 0; i < numClauses; i++) {
        int unsatisfiedLiterals = 0;
        int unit_literal = 0;
        int satisfied = 0;
        
        for (int j = 0; j < clauseSizes[i]; j++) {
            int literal = clauses[i][j];
            int var = abs(literal);
            
            if (var <= 0 || var > numVariables) continue;
            
            int value = assignment[var];
            
            if (value == 0) {  // 未赋值
                unsatisfiedLiterals++;
                unit_literal = literal;
            } else if ((literal > 0 && value == 1) || (literal < 0 && value == -1)) {
                satisfied = 1;
                break;
            }
        }
        
        if (!satisfied && unsatisfiedLiterals == 1) {
            *unitLiteral = unit_literal;
            return 1;
        }
    }
    return 0;
}

// 单元传播
int unitPropagation(int** clauses, int* clauseSizes, int numClauses, int* assignment) {
    int changed = 1;
    while (changed) {
        changed = 0;
        int unitLiteral;
        if (findUnitClause(clauses, clauseSizes, numClauses, assignment, &unitLiteral)) {
            int var = abs(unitLiteral);
            if (unitLiteral > 0) {
                assignment[var] = 1;
            } else {
                assignment[var] = -1;
            }
            changed = 1;
        }
    }
    
    // 检查是否有空子句（所有文字都被赋值且都不满足）
    for (int i = 0; i < numClauses; i++) {
        int all_assigned_and_false = 1;
        for (int j = 0; j < clauseSizes[i]; j++) {
            int literal = clauses[i][j];
            int var = abs(literal);
            int value = assignment[var];
            
            if (value == 0) {  // 有未赋值的文字
                all_assigned_and_false = 0;
                break;
            }
            
            if ((literal > 0 && value == 1) || (literal < 0 && value == -1)) {
                all_assigned_and_false = 0;
                break;
            }
        }
        if (all_assigned_and_false && clauseSizes[i] > 0) {
            return 0;  // 发现空子句（矛盾）
        }
    }
    
    return 1;
}

// 找到第一个未赋值的变量
int findUnassignedVariable(int* assignment, int numVariables) {
    for (int i = 1; i <= numVariables; i++) {
        if (assignment[i] == 0) {
            return i;
        }
    }
    return 0;
}

// DPLL递归函数
int dpll_recursive(int* assignment) {
    // 单元传播
    if (!unitPropagation(clauses, clauseSizes, numClauses, assignment)) {
        return 0;  // 不可满足 - 发现冲突
    }
    
    // 检查是否所有子句都满足
    if (isFormulaSatisfied(clauses, clauseSizes, numClauses, assignment)) {
        return 1;  // 可满足 - 找到解
    }
    
    // 选择变量进行分支
    int var = findUnassignedVariable(assignment, numVariables);
    if (var == 0) {
        // 没有未赋值变量 - 检查是否真的满足了所有子句
        return isFormulaSatisfied(clauses, clauseSizes, numClauses, assignment) ? 1 : 0;
    }
    
    // 尝试 var = true
    assignment[var] = 1;
    if (dpll_recursive(assignment)) {
        return 1;  // 找到解，直接返回
    }
    
    // 回溯，尝试 var = false
    assignment[var] = -1;
    if (dpll_recursive(assignment)) {
        return 1;  // 找到解，直接返回
    }
    
    // 如果都不行，回溯
    assignment[var] = 0;
    return 0;  // 不可满足
}

int solveSAT() {
    // 初始化变量赋值数组
    if (variableAssignment) {
        free(variableAssignment);
    }
    variableAssignment = (int*)calloc(numVariables + 1, sizeof(int));
    
    if (!variableAssignment) {
        printf("Memory allocation failed for variableAssignment\n");
        return -1; // 内存分配失败
    }
    
    printf("Starting DPLL with %d variables and %d clauses\n", numVariables, numClauses);
    
    int result = dpll_recursive(variableAssignment);
    
    printf("DPLL returned: %d\n", result);
    return result;
}