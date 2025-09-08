#ifndef SAT_DATA_STRUCTURES_H
#define SAT_DATA_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>

typedef int Literal;   // 文字, 正负之分
typedef int Variable;  // 变元

#define TRUE 1          // python风格的TRUE
#define FALSE 0
#define UNASSIGNED -1   // 没有赋值的状态

#define INIT_CAPACITY 10 // 动态数组初始容量

enum SatResult {
    SAT,        // 可满足
    UNSAT,      // 不可满足
    UNKNOWN     // 未知(超时等)
};

typedef struct {
    Literal* data;
    int size;           // 当前元素个数
    int capacity;       // 数组容量
} Clause;

typedef struct{
    Clause* data;
    int size;
    int capacity;
    int num_variables;  // 变量总数
    int num_clauses;    // 子句总数
} CNF;

// 变量赋值数组
typedef struct {
    int* values;        // 赋值数组 (1..num_variables)
    int num_variables;  // 变量总数
} Assignment;

// =========== 动态数组操作函数声明 ===========
// 动态数组操作函数声明
void init_dynamic_array(Clause* arr);
void push_dynamic_array(Clause* arr, void* element);
void free_dynamic_array(Clause* arr);
void clear_dynamic_array(Clause* arr);
int is_empty_dynamic_array(const Clause* arr);
void copy_dynamic_array(Clause* dest, const Clause* src);

// 重载
void init_dynamic_array(CNF* arr);
void push_dynamic_array(CNF* arr, void* element);
void free_dynamic_array(CNF* arr);
void clear_dynamic_array(CNF* arr);
int is_empty_dynamic_array(const CNF* arr);
void copy_dynamic_array(CNF* dest, const CNF* src);


// =========== 赋值操作函数 ===========
void init_assignment(Assignment* assignment, int num_variables);
void free_assignment(Assignment* assignment);
void copy_assignment(Assignment* dest, const Assignment* src);

// =========== CNF操作函数 ===========
int is_cnf_empty(const CNF* cnf);
int has_empty_clause(const CNF* cnf);

// =========== 判断是否为单元子句 ===========
int is_unit_clause(const Clause* clause);



#endif // SAT_DATA_STRUCTURES_H