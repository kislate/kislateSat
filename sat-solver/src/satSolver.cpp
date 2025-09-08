#include "satSolver.h"

// =========== 单元传播实现 ===========
int unit_propagate(CNF* cnf, Literal literal, Assignment* assignment) {
    // 记录赋值
    int var = (literal > 0) ? literal : -literal;
    assignment->values[var] = (literal > 0) ? TRUE : FALSE;
    
    // 创建新的CNF用于存储传播后的结果
    CNF new_cnf;
    init_dynamic_array(&new_cnf);
    new_cnf.num_variables = cnf->num_variables;
    
    for (int i = 0; i < cnf->size; i++) {
        Clause* clause = &cnf->data[i];
        int satisfied = FALSE;
        
        // 检查子句是否被literal满足
        for (int j = 0; j < clause->size; j++) {
            if (clause->data[j] == literal) {
                satisfied = TRUE;
                break;
            }
        }
        
        // 如果子句被满足，跳过
        if (satisfied) continue;
        
        // 创建新子句，删除-literal
        Clause new_clause;
        init_dynamic_array(&new_clause);
        
        for (int j = 0; j < clause->size; j++) {
            if (clause->data[j] != -literal) {
                push_dynamic_array(&new_clause, &clause->data[j]);
            }
        }
        
        // 如果遇到空子句，传播失败
        if (new_clause.size == 0) {
            free_dynamic_array(&new_clause);
            free_dynamic_array(&new_cnf);
            return FALSE;
        }
        
        push_dynamic_array(&new_cnf, &new_clause);
    }
    
    // 用新CNF替换原CNF
    free_dynamic_array(cnf);
    *cnf = new_cnf;
    
    return TRUE;
}

// =========== 文字选择策略实现 ===========
Literal select_literal_simple(const CNF* cnf) {
    // 简单策略：选择第一个子句的第一个文字
    if (cnf->size > 0 && cnf->data[0].size > 0) {
        return cnf->data[0].data[0];
    }
    return 0;
}

// 找出现次数最多的
Literal select_literal_occureMax(const CNF* cnf)
{
    if (cnf->size == 0) return 0;
    
    int* pos_count = (int*)calloc(cnf->num_variables + 1, sizeof(int));
    int* neg_count = (int*)calloc(cnf->num_variables + 1, sizeof(int));
    
    // 计算每个文字的出现次数
    for (int i = 0; i < cnf->size; i++) {
        Clause* clause = &cnf->data[i];
        for (int j = 0; j < clause->size; j++) {
            Literal lit = clause->data[j];
            int var = (lit > 0) ? lit : -lit;
            
            if (lit > 0) {
                pos_count[var]++;
            } else {
                neg_count[var]++;
            }
        }
    }
    
    // 找到出现次数最多的文字
    int max_count = -1;
    Literal best_literal = 0;
    
    for (int i = 1; i <= cnf->num_variables; i++) {
        if (pos_count[i] > max_count) {
            max_count = pos_count[i];
            best_literal = i;
        }
        if (neg_count[i] > max_count) {
            max_count = neg_count[i];
            best_literal = -i;
        }
    }
    
    free(pos_count);
    free(neg_count);
    
    return best_literal;
}

// 求第六个有奇效
Literal select_literal_jw(const CNF* cnf) {
    if (cnf->size == 0) return 0;
    
    double* pos_weights = (double*)calloc(cnf->num_variables + 1, sizeof(double));
    double* neg_weights = (double*)calloc(cnf->num_variables + 1, sizeof(double));
    
    // 计算Jeroslow-Wang启发式权重
    for (int i = 0; i < cnf->size; i++) {
        Clause* clause = &cnf->data[i];
        if (clause->size == 0) continue;
        
        double weight = pow(2.0, -clause->size);
        
        for (int j = 0; j < clause->size; j++) {
            Literal lit = clause->data[j];
            int var = (lit > 0) ? lit : -lit;
            
            if (lit > 0) {
                pos_weights[var] += weight;
            } else {
                neg_weights[var] += weight;
            }
        }
    }
    
    // 找到权重最大的文字
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

// =========== DPLL求解器实现 ===========
SatResult dpll_solve(CNF* cnf, Assignment* assignment) {
    // 单元传播循环
    while (TRUE) {
        int unit_found = FALSE;
        
        for (int i = 0; i < cnf->size; i++) {
            if (is_unit_clause(&cnf->data[i])) {
                // 发现单元子句，进行传播
                Literal unit_literal = cnf->data[i].data[0];
                
                if (!unit_propagate(cnf, unit_literal, assignment)) {
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
    
    // 检查是否有空子句
    if (has_empty_clause(cnf)) {
        return UNSAT;
    }
    
    // 如果CNF为空，所有子句都被满足
    if (is_cnf_empty(cnf)) {
        return SAT;
    }
    
    // 选择一个文字进行分支
    // Literal literal = select_literal_jw(cnf);
    // Literal literal = select_literal_occureMax(cnf);
    Literal literal;
    int rand_seed = rand() % 100;
    if (rand_seed < 0) literal = select_literal_simple(cnf);
    else if (rand_seed < 93) literal = select_literal_jw(cnf);
    else literal = select_literal_occureMax(cnf);

    if (literal == 0) {
        return UNSAT; // 没有可供选择的文字
    }
    
    int var = (literal > 0) ? literal : -literal;
    
    // 尝试正向赋值
    CNF cnf_backup;
    copy_dynamic_array(&cnf_backup, cnf);
    Assignment assign_backup;
    copy_assignment(&assign_backup, assignment);
    
    if (unit_propagate(cnf, var, assignment)) {
        SatResult result = dpll_solve(cnf, assignment);
        if (result == SAT) {
            free_dynamic_array(&cnf_backup);
            free_assignment(&assign_backup);
            return SAT;
        }
    }
    
    // 恢复状态并尝试负向赋值
    free_dynamic_array(cnf);
    copy_dynamic_array(cnf, &cnf_backup);
    free_assignment(assignment);
    copy_assignment(assignment, &assign_backup);
    
    if (unit_propagate(cnf, -var, assignment)) {
        SatResult result = dpll_solve(cnf, assignment);
        if (result == SAT) {
            free_dynamic_array(&cnf_backup);
            free_assignment(&assign_backup);
            return SAT;
        }
    }
    
    free_dynamic_array(&cnf_backup);
    free_assignment(&assign_backup);
    
    // 两个分支都失败
    return UNSAT;
}

// =========== 主求解接口实现 ===========
SatResult solve_sat(CNF* cnf, Assignment* assignment) {
    // 初始化赋值
    init_assignment(assignment, cnf->num_variables);
    
    // 创建CNF副本用于求解
    CNF cnf_copy;
    copy_dynamic_array(&cnf_copy, cnf);
    
    // 开始DPLL求解
    SatResult result = dpll_solve(&cnf_copy, assignment);
    
    free_dynamic_array(&cnf_copy);
    
    return result;
}
