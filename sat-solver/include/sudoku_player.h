#ifndef SUDOKU_PLAYER_H
#define SUDOKU_PLAYER_H

#include "sudoku_final.h"
#include "satSolver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 数独网格定义
#define SUDOKU_SIZE 9
#define EMPTY_CELL 0

// 数独玩家结构体
typedef struct {
    int grid[SUDOKU_SIZE][SUDOKU_SIZE];          // 当前玩家填写的数独
    int original[SUDOKU_SIZE][SUDOKU_SIZE];      // 原始题目(不可修改)
    int solution[SUDOKU_SIZE][SUDOKU_SIZE];      // 正确答案
    int player_filled[SUDOKU_SIZE][SUDOKU_SIZE]; // 标记哪些是玩家填写的
} SudokuPlayer;

// 函数声明
void init_sudoku_player(SudokuPlayer* player);
void free_sudoku_player(SudokuPlayer* player);
int load_sudoku_from_solution(SudokuPlayer* player, Assignment* assignment);
int load_sudoku_from_cnf(SudokuPlayer* player, CNF* cnf, Assignment* solution);
void display_sudoku_grid(SudokuPlayer* player, int show_solution);
void display_sudoku_with_errors(SudokuPlayer* player);
int player_fill_cell(SudokuPlayer* player, int row, int col, int value);
void give_random_hint(SudokuPlayer* player);
void give_specific_hint(SudokuPlayer* player, int row, int col);
int check_sudoku_completion(SudokuPlayer* player);
int validate_sudoku_input(int row, int col, int value);
void sudoku_player_interface(Assignment* assignment);
void sudoku_player_interface_with_cnf(CNF* cnf, Assignment* solution, const char* cnf_filename);

// 辅助函数
void clear_input_buffer();
void wait_for_player_enter();

#endif // SUDOKU_PLAYER_H
