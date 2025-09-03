#include "sat_solver.h"
#include <math.h>
#include <time.h>

// 全局变量用于跟踪求解状态
int dpll_call_count = 0;
int unit_propagation_count = 0;
int backtrack_count = 0;
time_t last_output_time = 0;

// 实时输出函数
void print_status_update() {
#ifdef DEBUG
    time_t current_time = time(NULL);
    if (current_time - last_output_time >= 2) { // 每2秒输出一次状态
        DEBUG_PRINT("求解中... DPLL调用次数: %d, 单元传播次数: %d, 回溯次数: %d\n", 
               dpll_call_count, unit_propagation_count, backtrack_count);
        DEBUG_FLUSH(); // 确保立即输出
        last_output_time = current_time;
    }
#endif
}

// =========== 动态数组操作实现 ===========

void init_literal_array(LiteralArray* arr) {
    arr->capacity = 4;
    arr->size = 0;
    arr->data = (Literal*)malloc(arr->capacity * sizeof(Literal));
}

void push_literal(LiteralArray* arr, Literal lit) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = (Literal*)realloc(arr->data, arr->capacity * sizeof(Literal));
    }
    arr->data[arr->size++] = lit;
}

void free_literal_array(LiteralArray* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void init_clause_array(ClauseArray* arr) {
    arr->capacity = 16;
    arr->size = 0;
    arr->data = (Clause*)malloc(arr->capacity * sizeof(Clause));
}

void push_clause(ClauseArray* arr, const Clause* clause) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = (Clause*)realloc(arr->data, arr->capacity * sizeof(Clause));
    }
    // 深拷贝子句
    init_literal_array(&arr->data[arr->size].literals);
    for (int i = 0; i < clause->literals.size; i++) {
        push_literal(&arr->data[arr->size].literals, clause->literals.data[i]);
    }
    arr->size++;
}

void free_clause_array(ClauseArray* arr) {
    for (int i = 0; i < arr->size; i++) {
        free_literal_array(&arr->data[i].literals);
    }
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

// =========== CNF操作实现 ===========

void init_cnf(CNF* cnf) {
    init_clause_array(&cnf->clauses);
    cnf->num_variables = 0;
    cnf->num_clauses = 0;
}

void free_cnf(CNF* cnf) {
    free_clause_array(&cnf->clauses);
    cnf->num_variables = 0;
    cnf->num_clauses = 0;
}

int load_cnf_from_file(CNF* cnf, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("无法打开文件: %s\n", filename);
        return 0;
    }
    
    char line[1024];
    int variables_found = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释行
        if (line[0] == 'c') continue;
        
        // 读取问题行
        if (line[0] == 'p') {
            char format[10];
            if (sscanf(line, "p %s %d %d", format, &cnf->num_variables, &cnf->num_clauses) != 3) {
                printf("无效的问题行格式\n");
                fclose(file);
                return 0;
            }
            if (strcmp(format, "cnf") != 0) {
                printf("不支持的格式: %s\n", format);
                fclose(file);
                return 0;
            }
            variables_found = 1;
            continue;
        }
        
        // 如果还没有读取到变量数，跳过
        if (!variables_found) continue;
        
        // 读取子句
        Clause clause;
        init_literal_array(&clause.literals);
        
        char* token = strtok(line, " \t\n");
        while (token) {
            int literal = atoi(token);
            if (literal == 0) break; // 子句结束
            push_literal(&clause.literals, literal);
            token = strtok(NULL, " \t\n");
        }
        
        if (clause.literals.size > 0) {
            push_clause(&cnf->clauses, &clause);
        }
        free_literal_array(&clause.literals);
    }
    
    fclose(file);
    
    DEBUG_PRINT("成功读取CNF文件:\n");
    DEBUG_PRINT("  变量数: %d\n", cnf->num_variables);
    DEBUG_PRINT("  子句数: %d (实际读取: %d)\n", cnf->num_clauses, cnf->clauses.size);
    
    return 1;
}

// =========== 赋值操作实现 ===========

void init_assignment(Assignment* assign, int num_variables) {
    assign->size = num_variables;
    assign->values = (int*)malloc((num_variables + 1) * sizeof(int)); // 1-indexed
    for (int i = 0; i <= num_variables; i++) {
        assign->values[i] = UNASSIGNED;
    }
}

void free_assignment(Assignment* assign) {
    free(assign->values);
    assign->values = NULL;
    assign->size = 0;
}

// =========== 辅助函数实现 ===========

int literal_value(Literal lit, const Assignment* assignment) {
    int var = (lit > 0) ? lit : -lit;
    int sign = (lit > 0) ? 1 : -1;
    
    if (var > assignment->size || assignment->values[var] == UNASSIGNED) {
        return UNASSIGNED;
    }
    
    return (assignment->values[var] == TRUE) ? (sign > 0 ? TRUE : FALSE) : (sign > 0 ? FALSE : TRUE);
}

int is_clause_satisfied(const Clause* clause, const Assignment* assignment) {
    for (int i = 0; i < clause->literals.size; i++) {
        if (literal_value(clause->literals.data[i], assignment) == TRUE) {
            return TRUE;
        }
    }
    return FALSE;
}

int is_clause_unsatisfied(const Clause* clause, const Assignment* assignment) {
    for (int i = 0; i < clause->literals.size; i++) {
        if (literal_value(clause->literals.data[i], assignment) != FALSE) {
            return FALSE;
        }
    }
    return TRUE;
}

int is_satisfied(const CNF* cnf, const Assignment* assignment) {
    for (int i = 0; i < cnf->clauses.size; i++) {
        if (!is_clause_satisfied(&cnf->clauses.data[i], assignment)) {
            return FALSE;
        }
    }
    return TRUE;
}

int has_unsatisfied_clause(const CNF* cnf, const Assignment* assignment) {
    for (int i = 0; i < cnf->clauses.size; i++) {
        if (is_clause_unsatisfied(&cnf->clauses.data[i], assignment)) {
            return TRUE;
        }
    }
    return FALSE;
}

int is_unit_clause(const Clause* clause, const Assignment* assignment, Literal* unit_literal) {
    int unassigned_count = 0;
    Literal unassigned_literal = 0;
    
    for (int i = 0; i < clause->literals.size; i++) {
        int val = literal_value(clause->literals.data[i], assignment);
        if (val == TRUE) {
            return FALSE; // 子句已满足
        } else if (val == UNASSIGNED) {
            unassigned_count++;
            unassigned_literal = clause->literals.data[i];
        }
    }
    
    if (unassigned_count == 1) {
        *unit_literal = unassigned_literal;
        return TRUE;
    }
    
    return FALSE;
}

Variable select_variable(const CNF* cnf, const Assignment* assignment) {
    // 简单的选择策略：选择第一个未赋值的变量
    for (int i = 1; i <= assignment->size; i++) {
        if (assignment->values[i] == UNASSIGNED) {
            return i;
        }
    }
    return 0; // 所有变量都已赋值
}

// =========== DPLL求解器实现（基于fromhusking逻辑）===========

// 传播函数：给定文字，修改CNF并更新赋值
int propagate(CNF* cnf, Literal literal, Assignment* assignment) {
    // 记录赋值
    int var = (literal > 0) ? literal : -literal;
    int value = (literal > 0) ? TRUE : FALSE;
    assignment->values[var] = value;
    
    CNF new_cnf;
    init_cnf(&new_cnf);
    new_cnf.num_variables = cnf->num_variables;
    
    for (int i = 0; i < cnf->clauses.size; i++) {
        Clause* clause = &cnf->clauses.data[i];
        int satisfied = FALSE;
        
        // 检查子句是否被literal满足
        for (int j = 0; j < clause->literals.size; j++) {
            if (clause->literals.data[j] == literal) {
                satisfied = TRUE;
                break;
            }
        }
        
        if (satisfied) {
            // 子句被满足，跳过（不添加到新CNF）
            continue;
        }
        
        // 子句未被满足，创建新子句并删除-literal
        Clause new_clause;
        init_literal_array(&new_clause.literals);
        
        for (int j = 0; j < clause->literals.size; j++) {
            if (clause->literals.data[j] != -literal) {
                push_literal(&new_clause.literals, clause->literals.data[j]);
            }
        }
        
        if (new_clause.literals.size == 0) {
            // 遇到空子句，传播失败
            free_literal_array(&new_clause.literals);
            free_cnf(&new_cnf);
            return FALSE;
        }
        
        push_clause(&new_cnf.clauses, &new_clause);
        free_literal_array(&new_clause.literals);
    }
    
    // 用新CNF替换原CNF
    free_clause_array(&cnf->clauses);
    cnf->clauses = new_cnf.clauses;
    cnf->num_clauses = new_cnf.clauses.size;
    
    return TRUE;
}

// 选择文字函数
Literal select_literal(const CNF* cnf) {
    // 策略：选择第一个子句的第一个文字
    if (cnf->clauses.size > 0 && cnf->clauses.data[0].literals.size > 0) {
        return cnf->clauses.data[0].literals.data[0];
    }
    return 0; // 没有可选择的文字
}

// 检查是否为单元子句
int is_unit_clause_simple(const Clause* clause) {
    return clause->literals.size == 1;
}

// 复制CNF
void copy_cnf(CNF* dest, const CNF* src) {
    init_cnf(dest);
    dest->num_variables = src->num_variables;
    dest->num_clauses = src->num_clauses;
    
    for (int i = 0; i < src->clauses.size; i++) {
        push_clause(&dest->clauses, &src->clauses.data[i]);
    }
}

SatResult dpll_solve(CNF* cnf, Assignment* assignment) {
    dpll_call_count++;
    print_status_update();
    
    // 单元传播循环
    while (TRUE) {
        int unit_found = FALSE;
        
        for (int i = 0; i < cnf->clauses.size; i++) {
            if (is_unit_clause_simple(&cnf->clauses.data[i])) {
                // 发现单元子句，进行传播
                Literal unit_literal = cnf->clauses.data[i].literals.data[0];
                unit_propagation_count++;
                
                if (!propagate(cnf, unit_literal, assignment)) {
                    return UNSAT; // 传播失败，冲突
                }
                
                unit_found = TRUE;
                break; // 重新开始查找单元子句
            }
        }
        
        if (!unit_found) {
            break; // 没有更多单元子句
        }
    }
    
    // 如果CNF为空，所有子句都被满足
    if (cnf->clauses.size == 0) {
        return SAT;
    }
    
    // 选择一个文字进行分支
    Literal literal = select_literal(cnf);
    if (literal == 0) {
        return UNSAT; // 没有可供选择的文字
    }
    
    int var = (literal > 0) ? literal : -literal;
    
    // 尝试正向赋值
    CNF cnf_true;
    copy_cnf(&cnf_true, cnf);
    Assignment assign_backup = *assignment;
    
    if (propagate(&cnf_true, var, assignment)) {
        SatResult result = dpll_solve(&cnf_true, assignment);
        if (result == SAT) {
            free_cnf(&cnf_true);
            return SAT;
        }
    }
    
    free_cnf(&cnf_true);
    
    // 恢复赋值并尝试负向赋值
    backtrack_count++;
    *assignment = assign_backup;
    CNF cnf_false;
    copy_cnf(&cnf_false, cnf);
    
    if (propagate(&cnf_false, -var, assignment)) {
        SatResult result = dpll_solve(&cnf_false, assignment);
        if (result == SAT) {
            free_cnf(&cnf_false);
            return SAT;
        }
    }
    
    free_cnf(&cnf_false);
    
    // 两个分支都失败
    backtrack_count++;
    *assignment = assign_backup;
    return UNSAT;
}

// =========== 输出结果实现 ===========

void save_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("无法创建输出文件: %s\n", filename);
        return;
    }
    
    if (result == SAT) {
        fprintf(file, "s 1\n");
        fprintf(file, "v ");
        for (int i = 1; i <= assignment->size; i++) {
            if (assignment->values[i] == TRUE) {
                fprintf(file, "%d ", i);
            } else if (assignment->values[i] == FALSE) {
                fprintf(file, "%d ", -i);
            }
        }
        fprintf(file, "\n");
    } else {
        fprintf(file, "s 0\n");
    }
    
    fprintf(file, "t %.0f\n", elapsed_time_ms);
    
    fclose(file);
}
