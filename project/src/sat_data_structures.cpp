#include "sat_data_structures.h"
// 全部用英文替换中文
// =========== 动态数组操作实现 ===========

// LiteralArray操作
void init_literal_array(LiteralArray* arr)
{
    // 初始化数组容量为4
    arr->capacity = 4;
    arr->size = 0;
    
    // 分配
    arr->data = (Literal*)malloc(arr->capacity * sizeof(Literal));
    if (!arr->data) 
    {
        // fprintf(stderr, "内存分配失败: init_literal_array\n");
        fprintf(stderr, "Memory Allocation Failed: init_literal_array\n");
        exit(1); // 强制终止, 因为有时候退不出来
    }
}

void push_literal(LiteralArray* arr, Literal lit)
{
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2; // 每次翻两倍
        arr->data = (Literal*)realloc(arr->data, arr->capacity * sizeof(Literal));
        if (!arr->data)
        {
            // fprintf(stderr, "内存重分配失败: push_literal\n");
            fprintf(stderr, "Memory Reallocation Failed: push_literal\n");
            exit(1);
        }
    }
    arr->data[arr->size++] = lit;
}

void free_literal_array(LiteralArray* arr)
{
    if (arr->data) {
        free(arr->data);
    }
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void clear_literal_array(LiteralArray* arr)
{
    arr->size = 0;
}

int is_empty_literal_array(const LiteralArray* arr)
{
    return arr->size == 0;
}

// ClauseArray操作: 和Literal的操作完全一样
void init_clause_array(ClauseArray* arr)
{
    arr->capacity = 16;
    arr->size = 0;
    arr->data = (Clause*)malloc(arr->capacity * sizeof(Clause));
    if (!arr->data) {
        // fprintf(stderr, "内存分配失败: init_clause_array\n");
        fprintf(stderr, "Memory Allocation Failed: init_clause_array\n");
        exit(1);
    }
}

void push_clause(ClauseArray* arr, const Clause* clause)
{
    if (arr->size >= arr->capacity)
    {
        arr->capacity *= 2;
        arr->data = (Clause*)realloc(arr->data, arr->capacity * sizeof(Clause));
        if (!arr->data)
        {
            // fprintf(stderr, "内存重分配失败: push_clause\n");
            fprintf(stderr, "Memory Reallocation Failed: push_clause\n");
            exit(1);
        }
    }
    // 深拷贝子句
    init_literal_array(&arr->data[arr->size].literals);
    for (int i = 0; i < clause->literals.size; i++)
    {
        push_literal(&arr->data[arr->size].literals, clause->literals.data[i]);
    }
    arr->size++;
}

void free_clause_array(ClauseArray* arr)
{
    // 先清理子句
    for (int i = 0; i < arr->size; i++) free_literal_array(&arr->data[i].literals);
    if (arr->data) free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void clear_clause_array(ClauseArray* arr)
{
    for (int i = 0; i < arr->size; i++) free_literal_array(&arr->data[i].literals);
}

int is_empty_clause_array(const ClauseArray* arr) 
{
    return arr->size == 0;
}

// =========== CNF公式操作实现 ===========

void init_cnf(CNF* cnf)
{
    init_clause_array(&cnf->clauses);
    cnf->num_variables = 0;
    cnf->num_clauses = 0;
}

void free_cnf(CNF* cnf)
{
    free_clause_array(&cnf->clauses);
    cnf->num_variables = 0;
    cnf->num_clauses = 0;
}

void clear_cnf(CNF* cnf)
{
    clear_clause_array(&cnf->clauses);
    cnf->num_clauses = 0;
}

void copy_cnf(CNF* dest, const CNF* src) {
    init_cnf(dest);
    dest->num_variables = src->num_variables;
    dest->num_clauses = src->num_clauses;
    
    for (int i = 0; i < src->clauses.size; i++) {
        push_clause(&dest->clauses, &src->clauses.data[i]);
    }
}

int is_cnf_empty(const CNF* cnf)
{
    return is_empty_clause_array(&cnf->clauses);
}

// =========== 子句操作实现 ===========

void init_clause(Clause* clause)
{
    init_literal_array(&clause->literals);
}

void free_clause(Clause* clause)
{
    free_literal_array(&clause->literals);
}

void copy_clause(Clause* dest, const Clause* src)
{
    init_literal_array(&dest->literals);
    for (int i = 0; i < src->literals.size; i++)
        push_literal(&dest->literals, src->literals.data[i]);
}

int is_clause_empty(const Clause* clause)
{
    return is_empty_literal_array(&clause->literals);
}

int is_unit_clause(const Clause* clause)
{
    return clause->literals.size == 1;
}

// =========== 赋值操作实现 ===========

void init_assignment(Assignment* assign, int num_variables)
{
    assign->size = num_variables;
    assign->values = (int*)malloc((num_variables + 1) * sizeof(int)); // 1-indexed
    if (!assign->values) {
        fprintf(stderr, "Memory Allocation Failed: init_assignment\n");
        exit(1); // 全部替换为强制退出
    }
    for (int i = 0; i <= num_variables; i++) assign->values[i] = UNASSIGNED;
}

void free_assignment(Assignment* assign) {
    if (assign->values) {
        free(assign->values);
    }
    assign->values = NULL;
    assign->size = 0;
}

void copy_assignment(Assignment* dest, const Assignment* src) {
    dest->size = src->size;
    dest->values = (int*)malloc((src->size + 1) * sizeof(int));
    if (!dest->values) {
        // fprintf(stderr, "内存分配失败: copy_assignment\n");
        fprintf(stderr, "Memory Allocation Failed: copy_assignment\n");
        exit(1);
    }
    for (int i = 0; i <= src->size; i++) {
        dest->values[i] = src->values[i];
    }
}

void clear_assignment(Assignment* assign) {
    for (int i = 0; i <= assign->size; i++) {
        assign->values[i] = UNASSIGNED;
    }
}

