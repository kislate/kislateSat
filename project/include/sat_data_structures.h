#ifndef SAT_DATA_STRUCTURES_H
#define SAT_DATA_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 不需要debug输出就注释掉
#define DEBUG

// =========== 基本数据类型定义 ===========
typedef int Literal;    // 文字, 正负之分
typedef int Variable;   // 变元

#define TRUE 1          // python风格的TRUE
#define FALSE 0         
#define UNASSIGNED -1   // 没有赋值的状态

// =========== 动态数组结构 - 拿来替代vector ===========
typedef struct {
    Literal* data;      // 存储文字的动态数组(int)
    int size;           // 当前元素个数
    int capacity;       // 数组容量
} LiteralArray;

// =========== 子句结构 ===========
typedef struct {
    LiteralArray literals;  // 子句中的文字数组
} Clause;

// =========== 子句数组 ===========
typedef struct {
    Clause* data;       // 存储子句的动态数组
    int size;           // 当前子句个数
    int capacity;       // 数组容量
} ClauseArray;

// =========== CNF公式结构 ===========
typedef struct {
    ClauseArray clauses;    // CNF公式的所有子句
    int num_variables;      // 变量总数
    int num_clauses;        // 子句总数
} CNF;

// =========== 赋值结构 ===========
typedef struct {
    int* values;        // 变量赋值数组：TRUE/FALSE/UNASSIGNED
    int size;           // 变量数量
} Assignment;

// =========== 求解结果枚举 ===========
typedef enum {
    SAT,        // 可满足
    UNSAT,      // 不可满足
    UNKNOWN     // 未知(超时等)
} SatResult;

// =========== 动态数组操作函数声明 ===========
// 后续可以考虑把他们两个合二为一
// LiteralArray操作

// 初始化数组
void init_literal_array(LiteralArray* arr);                
// cos一下.push_back();
void push_literal(LiteralArray* arr, Literal lit);         
// 释放空间
void free_literal_array(LiteralArray* arr);                
// 清空
void clear_literal_array(LiteralArray* arr);               
// 判空
int is_empty_literal_array(const LiteralArray* arr);       

// 和上面几乎一模一样
void init_clause_array(ClauseArray* arr);                  // 初始化子句数组
void push_clause(ClauseArray* arr, const Clause* clause);  // 添加子句到数组
void free_clause_array(ClauseArray* arr);                  // 释放子句数组内存
void clear_clause_array(ClauseArray* arr);                 // 清空子句数组
int is_empty_clause_array(const ClauseArray* arr);         // 检查子句数组是否为空

// =========== CNF公式操作函数声明 ===========

// 初始化CNF
void init_cnf(CNF* cnf);    
// 释放内存
void free_cnf(CNF* cnf);                 
// 清空              
void clear_cnf(CNF* cnf);            
// 从文件加载cnf
int load_cnf_from_file(CNF* cnf, const char* filename); 
// 从 src 中读取cnf给dest
void copy_cnf(CNF* dest, const CNF* src);          
// 基础的判空检查    
int is_cnf_empty(const CNF* cnf);                      

// =========== 子句操作函数声明 ===========
// 这里只是封装了一下literals, 经典左右脑互博了
void init_clause(Clause* clause);                      
void free_clause(Clause* clause);                      
// 判空
int is_clause_empty(const Clause* clause);  

// 复制子句
void copy_clause(Clause* dest, const Clause* src);    
        
// 检查是否为单元子句
int is_unit_clause(const Clause* clause);             

// =========== 赋值操作函数声明 ===========
// 初始化赋值结构
void init_assignment(Assignment* assign, int num_variables); 
// 释放赋值结构内存 
void free_assignment(Assignment* assign);                     
// 复制赋值结构
void copy_assignment(Assignment* dest, const Assignment* src); 
// 清空所有赋值
void clear_assignment(Assignment* assign);                    


#endif // SAT_DATA_STRUCTURES_H
// 6, 唐完了, 不如写个vector.h