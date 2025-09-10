#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include "dynamic.h"
#include <time.h>
#include <math.h>

// =========== 单元传播 ===========
int unit_propagate(CNF* cnf, Literal literal, Assignment* assignment);

// =========== 预处理优化 ===========
int pure_literal_elimination(CNF* cnf, Assignment* assignment);

// =========== 文字选择策略 ===========
Literal select_literal_simple(const CNF* cnf);
Literal select_literal_jw(const CNF* cnf);
Literal select_literal_mom(const CNF* cnf);  // MOM启发式
Literal select_literal_occureMax(const CNF* cnf);  // 出现次数最多启发式

// =========== DPLL求解器 ===========
SatResult dpll_solve(CNF* cnf, Assignment* assignment, int solve_mode = 0);

// =========== 主求解接口 ===========
SatResult solve_sat(CNF* cnf, Assignment* assignment, int solve_mode = 0);

#endif // SAT_SOLVER_H
