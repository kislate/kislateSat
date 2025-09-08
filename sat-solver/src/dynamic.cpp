#include "dynamic.h"
#include <string.h>

// =========== 动态数组操作函数实现 ===========
// 初始化动态数组
void init_dynamic_array(Clause* arr) {
    arr->data = (Literal*)malloc(INIT_CAPACITY * sizeof(Literal));
    arr->size = 0;
    arr->capacity = INIT_CAPACITY;
}

void init_dynamic_array(CNF* arr) {
    arr->data = (Clause*)malloc(INIT_CAPACITY * sizeof(Clause));
    arr->size = 0;
    arr->capacity = INIT_CAPACITY;
    arr->num_variables = 0;
    arr->num_clauses = 0;
}

// 添加元素到动态数组
void push_dynamic_array(Clause* arr, void* element) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = (Literal*)realloc(arr->data, arr->capacity * sizeof(Literal));
    }
    arr->data[arr->size++] = *(Literal*)element;
}

void push_dynamic_array(CNF* arr, void* element) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = (Clause*)realloc(arr->data, arr->capacity * sizeof(Clause));
    }
    arr->data[arr->size++] = *(Clause*)element;
    arr->num_clauses = arr->size; // 更新子句数量
}

// 释放动态数组内存
void free_dynamic_array(Clause* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void free_dynamic_array(CNF* arr) {
    for (int i = 0; i < arr->size; i++) {
        free_dynamic_array(&arr->data[i]); // 释放每个子句的内存
    }
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
    arr->num_variables = 0;
    arr->num_clauses = 0;
}

// 清空动态数组(保留容量)
void clear_dynamic_array(Clause* arr) {
    arr->size = 0;
}

void clear_dynamic_array(CNF* arr) {
    for (int i = 0; i < arr->size; i++) {
        clear_dynamic_array(&arr->data[i]);
    }
    arr->size = 0;
    arr->num_clauses = 0;
}

// 判断动态数组是否为空
int is_empty_dynamic_array(const Clause* arr) {
    return arr->size == 0;
}

int is_empty_dynamic_array(const CNF* arr) {
    return arr->size == 0;
}
    
// 复制动态数组 (用memcpy 优化)
void copy_dynamic_array(Clause* dest, const Clause* src) {
    // 直接分配所需内存
    dest->data = (Literal*)malloc(src->capacity * sizeof(Literal));
    dest->size = src->size;
    dest->capacity = src->capacity;
    
    // 使用memcpy批量复制
    memcpy(dest->data, src->data, src->size * sizeof(Literal));
}

void copy_dynamic_array(CNF* dest, const CNF* src) {
    // 直接分配所需内存
    dest->data = (Clause*)malloc(src->capacity * sizeof(Clause));
    dest->size = src->size;
    dest->capacity = src->capacity;
    dest->num_variables = src->num_variables;
    dest->num_clauses = src->num_clauses;
    
    // 复制每个子句
    for (int i = 0; i < src->size; i++) {
        copy_dynamic_array(&dest->data[i], &src->data[i]);
    }
}

// =========== 判断是否为单元子句 ===========
int is_unit_clause(const Clause* clause) {
    return clause->size == 1;
}

// =========== 赋值操作函数实现 ===========
void init_assignment(Assignment* assignment, int num_variables) {
    assignment->num_variables = num_variables;
    assignment->values = (int*)malloc((num_variables + 1) * sizeof(int));
    for (int i = 0; i <= num_variables; i++) {
        assignment->values[i] = UNASSIGNED;
    }
}

void free_assignment(Assignment* assignment) {
    free(assignment->values);
    assignment->values = NULL;
    assignment->num_variables = 0;
}

void copy_assignment(Assignment* dest, const Assignment* src) {
    init_assignment(dest, src->num_variables);
    memcpy(dest->values, src->values, (src->num_variables + 1) * sizeof(int));
}

// =========== CNF操作函数实现 ===========
int is_cnf_empty(const CNF* cnf) {
    return cnf->size == 0;
}

int has_empty_clause(const CNF* cnf) {
    for (int i = 0; i < cnf->size; i++) {
        if (cnf->data[i].size == 0) {
            return TRUE;
        }
    }
    return FALSE;
}