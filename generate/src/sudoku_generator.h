#ifndef SUDOKU_GENERATOR_H
#define SUDOKU_GENERATOR_H

void generateCompleteSudoku(int grid[9][9]);
void digHoles(int grid[9][9], int holes);
void printSudoku(int grid[9][9]);
void saveSudokuToCNF(int grid[9][9], const char* filename);

#endif