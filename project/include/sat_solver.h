#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include "sat_data_structures.h"
#include <time.h>

// 不需要debug输出就注释掉
#define DEBUG

// 调试宏定义 - 如果在main.cpp中已定义DEBUG，则启用调试输出
#ifdef DEBUG
    #define DEBUG_PRINT(...) printf(__VA_ARGS__)
    #define DEBUG_FLUSH() fflush(stdout)
#else
    #define DEBUG_PRINT(...) 
    #define DEBUG_FLUSH()
#endif

// 外部变量声明（用于跟踪求解状态）
extern int dpll_call_count;
extern int unit_propagation_count;
extern int backtrack_count;
extern time_t last_output_time;

// DPLL求解器函数声明
SatResult dpll_solve(CNF* cnf, Assignment* assignment);

// DPLL算法核心函数
int unitPropagate(CNF* cnf, Literal literal, Assignment* assignment);
Literal select_literal(const CNF* cnf);
Variable select_variable(const CNF* cnf, const Assignment* assignment);

// 输出结果
void save_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms);

// 实时状态输出
void print_status_update(void);

#endif // SAT_SOLVER_H
