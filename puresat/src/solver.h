#ifndef SOLVER_H
#define SOLVER_H

extern int* variableAssignment;  // 变量赋值数组

int solveSAT();
int dpll(int** clauses, int* clauseSizes, int numClauses, int numVariables);

#endif
