# 变元行索引优化方案

## 概述
通过添加数组来记录每个变元（正文字和负文字）在哪些子句中出现过，可以显著提高DPLL算法的性能，避免每次都遍历所有子句来查找包含特定文字的子句。

## 优化原理
当前实现中，每次需要查找包含某个文字的子句时，都要遍历整个子句列表。通过预建索引，可以直接定位到相关子句，时间复杂度从O(m)降低到O(1)。

## 数据结构设计

### 1. 在 `sat_data_structures.h` 中添加新结构体：

```c
// 子句索引结构 - 记录每个变元出现在哪些子句中
typedef struct {
    int* clause_indices;    // 包含该文字的子句索引数组
    int size;              // 当前数组中的元素数量
    int capacity;          // 数组容量
} ClauseIndexArray;

// 变元索引结构 - 为每个变元的正负文字维护子句索引
typedef struct {
    ClauseIndexArray* positive_occurrences;  // positive_occurrences[i] 存储变元i的正文字出现的子句
    ClauseIndexArray* negative_occurrences;  // negative_occurrences[i] 存储变元i的负文字出现的子句
    int num_variables;                       // 变元数量
} VariableIndex;
```

### 2. 在 `CNF` 结构体中添加索引字段：

```c
typedef struct {
    ClauseArray clauses;
    int num_variables;
    int num_clauses;
    VariableIndex var_index;  // 新增：变元索引
} CNF;
```

## 实现方案

### 1. 基础操作函数（在 `sat_data_structures.cpp` 中）

```c
// 初始化子句索引数组
void init_clause_index_array(ClauseIndexArray* array) {
    array->clause_indices = NULL;
    array->size = 0;
    array->capacity = 0;
}

// 释放子句索引数组
void free_clause_index_array(ClauseIndexArray* array) {
    if (array->clause_indices) {
        free(array->clause_indices);
        array->clause_indices = NULL;
    }
    array->size = 0;
    array->capacity = 0;
}

// 向子句索引数组添加元素
void push_clause_index(ClauseIndexArray* array, int clause_index) {
    if (array->size >= array->capacity) {
        array->capacity = (array->capacity == 0) ? 4 : array->capacity * 2;
        array->clause_indices = (int*)realloc(array->clause_indices, 
                                             array->capacity * sizeof(int));
        if (!array->clause_indices) {
            fprintf(stderr, "Memory allocation failed for clause index array\n");
            exit(1);
        }
    }
    array->clause_indices[array->size++] = clause_index;
}

// 初始化变元索引
void init_variable_index(VariableIndex* index, int num_variables) {
    index->num_variables = num_variables;
    index->positive_occurrences = (ClauseIndexArray*)malloc((num_variables + 1) * sizeof(ClauseIndexArray));
    index->negative_occurrences = (ClauseIndexArray*)malloc((num_variables + 1) * sizeof(ClauseIndexArray));
    
    if (!index->positive_occurrences || !index->negative_occurrences) {
        fprintf(stderr, "Memory allocation failed for variable index\n");
        exit(1);
    }
    
    for (int i = 0; i <= num_variables; i++) {
        init_clause_index_array(&index->positive_occurrences[i]);
        init_clause_index_array(&index->negative_occurrences[i]);
    }
}

// 释放变元索引
void free_variable_index(VariableIndex* index) {
    if (index->positive_occurrences) {
        for (int i = 0; i <= index->num_variables; i++) {
            free_clause_index_array(&index->positive_occurrences[i]);
        }
        free(index->positive_occurrences);
        index->positive_occurrences = NULL;
    }
    
    if (index->negative_occurrences) {
        for (int i = 0; i <= index->num_variables; i++) {
            free_clause_index_array(&index->negative_occurrences[i]);
        }
        free(index->negative_occurrences);
        index->negative_occurrences = NULL;
    }
    
    index->num_variables = 0;
}

// 构建变元索引 - 在加载CNF后调用
void build_variable_index(CNF* cnf) {
    init_variable_index(&cnf->var_index, cnf->num_variables);
    
    // 遍历所有子句，建立索引
    for (int clause_idx = 0; clause_idx < cnf->clauses.size; clause_idx++) {
        Clause* clause = &cnf->clauses.data[clause_idx];
        
        for (int lit_idx = 0; lit_idx < clause->literals.size; lit_idx++) {
            Literal lit = clause->literals.data[lit_idx];
            int var = (lit > 0) ? lit : -lit;
            
            if (lit > 0) {
                // 正文字
                push_clause_index(&cnf->var_index.positive_occurrences[var], clause_idx);
            } else {
                // 负文字
                push_clause_index(&cnf->var_index.negative_occurrences[var], clause_idx);
            }
        }
    }
}
```

### 2. 修改CNF初始化和清理函数

```c
// 在 init_cnf 中初始化索引
void init_cnf(CNF* cnf) {
    init_clause_array(&cnf->clauses);
    cnf->num_variables = 0;
    cnf->num_clauses = 0;
    // 索引将在build_variable_index中初始化
}

// 在 free_cnf 中清理索引
void free_cnf(CNF* cnf) {
    free_clause_array(&cnf->clauses);
    free_variable_index(&cnf->var_index);  // 新增
    cnf->num_variables = 0;
    cnf->num_clauses = 0;
}
```

### 3. 在fileop.cpp中的load_cnf_from_file函数末尾添加：

```c
int load_cnf_from_file(CNF* cnf, const char* filename) {
    // ... 现有的加载代码 ...
    
    fclose(file);
    
    // 构建变元索引（新增）
    build_variable_index(cnf);
    
    #ifdef DEBUG
    printf("Successfully built variable index\n");
    printf("  Variables: %d, Clauses: %d\n", cnf->num_variables, cnf->clauses.size);
    #endif
    
    return 1;
}
```

### 4. 优化DPLL算法中的查找操作

#### 4.1 优化单元传播中的子句查找：

```c
// 替换原来的遍历所有子句的代码
int unit_propagate_optimized(CNF* cnf, Assignment* assignment) {
    int propagated = 0;
    
    do {
        propagated = 0;
        
        // 遍历所有子句查找单元子句
        for (int i = 0; i < cnf->clauses.size; i++) {
            Clause* clause = &cnf->clauses.data[i];
            
            // ... 单元子句检测代码 ...
            
            if (is_unit && unit_literal != 0) {
                // 设置单元文字
                int var = (unit_literal > 0) ? unit_literal : -unit_literal;
                int value = (unit_literal > 0) ? TRUE : FALSE;
                assignment->values[var] = value;
                
                // 使用索引快速更新相关子句
                propagate_assignment_optimized(cnf, assignment, var, value);
                propagated++;
                break;
            }
        }
    } while (propagated > 0);
    
    return 1;
}

// 新增：使用索引快速传播赋值
void propagate_assignment_optimized(CNF* cnf, Assignment* assignment, int var, int value) {
    // 根据赋值更新相关子句
    ClauseIndexArray* satisfied_clauses;
    ClauseIndexArray* affected_clauses;
    
    if (value == TRUE) {
        satisfied_clauses = &cnf->var_index.positive_occurrences[var];
        affected_clauses = &cnf->var_index.negative_occurrences[var];
    } else {
        satisfied_clauses = &cnf->var_index.negative_occurrences[var];
        affected_clauses = &cnf->var_index.positive_occurrences[var];
    }
    
    // 标记满足的子句（可以用位图或状态数组）
    for (int i = 0; i < satisfied_clauses->size; i++) {
        int clause_idx = satisfied_clauses->clause_indices[i];
        // 标记子句已满足
    }
    
    // 处理受影响的子句（移除对应文字）
    for (int i = 0; i < affected_clauses->size; i++) {
        int clause_idx = affected_clauses->clause_indices[i];
        // 在子句中移除该文字
    }
}
```

#### 4.2 优化变量选择启发式：

```c
// 优化Jeroslow-Wang启发式
Literal select_literal_jw_optimized(const CNF* cnf) {
    double* pos_weights = (double*)calloc(cnf->num_variables + 1, sizeof(double));
    double* neg_weights = (double*)calloc(cnf->num_variables + 1, sizeof(double));
    
    // 使用索引直接计算权重
    for (int var = 1; var <= cnf->num_variables; var++) {
        // 计算正文字权重
        ClauseIndexArray* pos_clauses = &cnf->var_index.positive_occurrences[var];
        for (int i = 0; i < pos_clauses->size; i++) {
            int clause_idx = pos_clauses->clause_indices[i];
            Clause* clause = &cnf->clauses.data[clause_idx];
            double weight = pow(2.0, -clause->literals.size);
            pos_weights[var] += weight;
        }
        
        // 计算负文字权重
        ClauseIndexArray* neg_clauses = &cnf->var_index.negative_occurrences[var];
        for (int i = 0; i < neg_clauses->size; i++) {
            int clause_idx = neg_clauses->clause_indices[i];
            Clause* clause = &cnf->clauses.data[clause_idx];
            double weight = pow(2.0, -clause->literals.size);
            neg_weights[var] += weight;
        }
    }
    
    // 选择最佳文字
    double max_score = -1.0;
    Literal best_literal = 0;
    
    for (int i = 1; i <= cnf->num_variables; i++) {
        if (pos_weights[i] > max_score) {
            max_score = pos_weights[i];
            best_literal = i;
        }
        if (neg_weights[i] > max_score) {
            max_score = neg_weights[i];
            best_literal = -i;
        }
    }
    
    free(pos_weights);
    free(neg_weights);
    return best_literal;
}
```

## 性能提升预期

1. **单元传播优化**：从O(m×k)降低到O(变元度数)，通常有5-10倍性能提升
2. **变量选择优化**：从O(n×m×k)降低到O(n×平均度数)，通常有10-50倍性能提升
3. **总体性能**：对于大型CNF实例，预期有3-20倍的性能提升

## 内存开销

- 每个变元需要两个ClauseIndexArray（正负文字）
- 总额外内存：约O(子句数×平均子句长度)
- 通常增加20%-50%的内存使用，但性能提升远大于内存开销

## 实现建议

1. **分阶段实现**：先实现基础结构，再逐步优化各个算法组件
2. **调试支持**：添加DEBUG宏控制的索引验证代码
3. **性能测试**：在大型CNF实例上对比优化前后的性能
4. **内存管理**：确保所有动态内存都被正确释放

## 兼容性

这个优化方案对现有代码的修改最小：
- 不改变现有的数据结构定义，只是扩展
- 不影响现有的算法逻辑，只是提供更快的查找方法
- 可以通过编译宏控制是否启用优化
