#include "sat_solver.h"
#include <math.h>

// 全局变量用于跟踪求解状态
int dpll_call_count = 0;
int unit_propagation_count = 0;
int backtrack_count = 0;
time_t last_output_time = 0;

// 告诉我你还活着
void print_status_update() {
#ifdef DEBUG
    time_t current_time = time(NULL);
    if (current_time - last_output_time >= 2) { // 每2秒输出一次状态
        // DEBUG_PRINT("求解中... DPLL调用次数: %d, 单元传播次数: %d, 回溯次数: %d\n", 
        //        dpll_call_count, unit_propagation_count, backtrack_count);
        DEBUG_PRINT("Solving... DPLL Calls: %d, Unit Propagations: %d, Backtracks: %d\n", 
               dpll_call_count, unit_propagation_count, backtrack_count);
        DEBUG_FLUSH(); // 确保立即输出
        last_output_time = current_time;
    }
#endif
}

// =========== DPLL求解器实现===========

// 传播函数：给定文字，修改CNF并更新赋值
int unitPropagate(CNF* cnf, Literal literal, Assignment* assignment) {
    // 记录赋值
    int var = (literal > 0) ? literal : -literal;
    // int value = (literal > 0) ? TRUE : FALSE;
    assignment->values[var] = (literal > 0) ? TRUE : FALSE;
    
    // 开始传播
    CNF new_cnf;
    init_cnf(&new_cnf);
    new_cnf.num_variables = cnf->num_variables;
    
    for (int i = 0; i < cnf->clauses.size; i++) {
        Clause* clause = &cnf->clauses.data[i];
        int satisfied = FALSE;
        
        // 检查子句是否被literal满足
        for (int j = 0; j < clause->literals.size; j++)
        {
            if (clause->literals.data[j] == literal) {
                satisfied = TRUE;
                break;
            }
        }
        
        // 子句满足
        if (satisfied) continue;
        
        // 子句未被满足，创建新子句并删除-literal
        Clause new_clause;
        init_literal_array(&new_clause.literals);
        
        // 十字链表是否可以优化
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


// =========== 赋值选择函数 ===========
// 选择文字函数
Literal select_literal(const CNF* cnf)
{
    // 策略：选择第一个子句的第一个文字
    if (cnf->clauses.size > 0 && cnf->clauses.data[0].literals.size > 0) {
        return cnf->clauses.data[0].literals.data[0];
    }
    return 0; // 没有可选择的文字
}

// 选择变量函数(目前没有用这个)
Variable select_variable(const CNF* cnf, const Assignment* assignment)
{
    // 简单的选择策略：选择第一个未赋值的变量
    for (int i = 1; i <= assignment->size; i++) {
        if (assignment->values[i] == UNASSIGNED) {
            return i;
        }
    }
    return 0; // 所有变量都已赋值
}

// Jeroslow-Wang 启发式 - 优化版本
Literal select_literal_jw(const CNF* cnf)
{
    if (cnf->clauses.size == 0) return 0;

    double* pos_weights = (double*)calloc(cnf->num_variables + 1, sizeof(double));
    double* neg_weights = (double*)calloc(cnf->num_variables + 1, sizeof(double));

    // 只遍历一次所有子句和文字
    for (int i = 0; i < cnf->clauses.size; i++)
    {
        Clause* clause = &cnf->clauses.data[i];
        int clause_size = clause->literals.size;
        if (clause_size == 0) continue;

        double weight = pow(2.0, -clause_size);  // 计算一次权重

        // 遍历子句中的每个文字，累加权重
        for (int j = 0; j < clause->literals.size; j++)
        {
            Literal lit = clause->literals.data[j];
            int var = (lit > 0) ? lit : -lit;
            
            if (lit > 0) pos_weights[var] += weight;
            else neg_weights[var] += weight;
        }
    }

    // 找到权重最大的文字
    double max_score = -1.0;
    Literal best_literal = 0;

    for (int i = 1; i <= cnf->num_variables; i++)
    {
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

SatResult dpll_solve(CNF* cnf, Assignment* assignment) 
{
    dpll_call_count++;
    print_status_update(); // 调试输出
    
    // 单元传播循环
    while (TRUE) {
        int unit_found = FALSE;
        
        for (int i = 0; i < cnf->clauses.size; i++)
        {
            if (is_unit_clause(&cnf->clauses.data[i]))
            {
                // 发现单元子句，进行传播
                Literal unit_literal = cnf->clauses.data[i].literals.data[0];
                unit_propagation_count++;
                
                if (!unitPropagate(cnf, unit_literal, assignment))
                {
                    return UNSAT; // 传播失败，冲突
                }
                
                unit_found = TRUE;
                break; // 重新开始查找单元子句
            }
        }
        
        if (!unit_found)
        {
            break; // 没有更多单元子句
        }
    }
    
    // 如果CNF为空，所有子句都被满足
    if (is_cnf_empty(cnf))
    {
        return SAT;
    }
    
    // 选择一个文字进行分支
    Literal literal = select_literal_jw(cnf);
    if (literal == 0)
    {
        return UNSAT; // 没有可供选择的文字
    }
    
    int var = (literal > 0) ? literal : -literal;
    
    // 尝试正向赋值
    CNF cnf_true;
    copy_cnf(&cnf_true, cnf);
    Assignment assign_backup;
    copy_assignment(&assign_backup, assignment);
    
    if (unitPropagate(&cnf_true, var, assignment))
    {
        SatResult result = dpll_solve(&cnf_true, assignment);
        if (result == SAT)
        {
            free_cnf(&cnf_true);
            free_assignment(&assign_backup);
            return SAT;
        }
    }
    
    free_cnf(&cnf_true);
    
    // 恢复赋值并尝试负向赋值
    backtrack_count++;
    free_assignment(assignment);
    *assignment = assign_backup;
    CNF cnf_false;
    copy_cnf(&cnf_false, cnf);
    
    if (unitPropagate(&cnf_false, -var, assignment))
    {
        SatResult result = dpll_solve(&cnf_false, assignment);
        if (result == SAT)
        {
            free_cnf(&cnf_false);
            return SAT;
        }
    }
    
    free_cnf(&cnf_false);
    
    // 两个分支都失败
    backtrack_count++;
    return UNSAT;
}
