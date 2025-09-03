#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "sat_data_structures.h"

// 数独网格大小
#define SUDOKU_SIZE 9
#define SUDOKU_CELLS 81

// 数独网格结构
typedef struct {
    int grid[SUDOKU_SIZE][SUDOKU_SIZE];
    int filled_cells;  // 已填充的格子数量
} SudokuGrid;

// 数独生成相关函数
void init_sudoku_grid(SudokuGrid* sudoku);
int is_valid_placement(const SudokuGrid* sudoku, int row, int col, int num);
int solve_sudoku_backtrack(SudokuGrid* sudoku);
void generate_full_sudoku(SudokuGrid* sudoku);
void create_puzzle_by_digging(SudokuGrid* sudoku, int holes_count);
void print_sudoku_grid(const SudokuGrid* sudoku);

// 数独文件保存功能
void save_sudoku_ss_format(const SudokuGrid* sudoku, const char* filename);
void save_sudoku_cnf_format(const SudokuGrid* sudoku, const char* filename);

// 数独转CNF相关函数
void sudoku_to_cnf(const SudokuGrid* sudoku, CNF* cnf);
int get_variable_number(int row, int col, int num);  // 获取变量编号
void add_sudoku_constraints(CNF* cnf);  // 添加数独约束

// 数独求解主函数
int generate_and_solve_sudoku();

#endif // SUDOKU_H
