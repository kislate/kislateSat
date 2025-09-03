#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// 基本数据类型定义
typedef int Literal;
typedef int Variable;

#define TRUE 1
#define FALSE 0
#define UNASSIGNED -1

// 动态数组结构 - 替代vector
typedef struct {
    Literal* data;
    int size;
    int capacity;
} LiteralArray;

// 子句结构
typedef struct {
    LiteralArray literals;
} Clause;

// 子句数组
typedef struct {
    Clause* data;
    int size;
    int capacity;
} ClauseArray;

// CNF公式结构
typedef struct {
    ClauseArray clauses;
    int num_variables;
    int num_clauses;
} CNF;

// 赋值结构
typedef struct {
    int* values;  // 变量赋值：TRUE/FALSE/UNASSIGNED
    int size;     // 变量数量
} Assignment;

// 求解结果
typedef enum {
    SAT,
    UNSAT,
    UNKNOWN
} SatResult;

// 函数声明
// 动态数组操作
void init_literal_array(LiteralArray* arr);
void push_literal(LiteralArray* arr, Literal lit);
void free_literal_array(LiteralArray* arr);

void init_clause_array(ClauseArray* arr);
void push_clause(ClauseArray* arr, const Clause* clause);
void free_clause_array(ClauseArray* arr);

// CNF操作
void init_cnf(CNF* cnf);
void free_cnf(CNF* cnf);
int load_cnf_from_file(CNF* cnf, const char* filename);

// 赋值操作
void init_assignment(Assignment* assign, int num_variables);
void free_assignment(Assignment* assign);

// DPLL求解器
SatResult dpll_solve(CNF* cnf, Assignment* assignment);

// 辅助函数
int propagate(CNF* cnf, Literal literal, Assignment* assignment);
Literal select_literal(const CNF* cnf);
void copy_cnf(CNF* dest, const CNF* src);
int is_unit_clause_simple(const Clause* clause);

// 输出结果
void save_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms);

#endif
