#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 函数声明
void generateCompleteSudoku(int grid[9][9]);
void digHoles(int grid[9][9], int holes);
void printSudoku(int grid[9][9]);
void saveSudokuToCNF(int grid[9][9], const char* filename);
int solveSudokuFromCNF(const char* cnf_filename, int solution[9][9]);

int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    if (argc < 2) {
        printf("Usage: %s <command> [options]\n", argv[0]);
        printf("Commands:\n");
        printf("  generate <holes>     - Generate a Sudoku puzzle (holes: 20-60)\n");
        printf("  solve <cnf_file>     - Solve a Sudoku CNF file\n");
        return 1;
    }
    
    if (strcmp(argv[1], "generate") == 0 && argc == 3) {
        int holes = atoi(argv[2]);
        if (holes < 20 || holes > 60) {
            printf("Holes should be between 20 and 60\n");
            return 1;
        }
        
        // 生成完整数独
        int sudoku[9][9];
        printf("Generating complete Sudoku...\n");
        generateCompleteSudoku(sudoku);
        printf("Complete Sudoku:\n");
        printSudoku(sudoku);
        
        // 挖洞
        printf("\nDigging %d holes...\n", holes);
        digHoles(sudoku, holes);
        printf("Puzzle:\n");
        printSudoku(sudoku);
        
        // 保存为CNF文件
        saveSudokuToCNF(sudoku, "sudoku_puzzle.cnf");
        printf("Generated CNF file: sudoku_puzzle.cnf\n");
        
    } else if (strcmp(argv[1], "solve") == 0 && argc == 3) {
        char* cnf_file = argv[2];
        int solution[9][9];
        
        printf("Solving Sudoku from CNF file: %s\n", cnf_file);
        int result = solveSudokuFromCNF(cnf_file, solution);
        
        if (result) {
            printf("Solution found:\n");
            printSudoku(solution);
        } else {
            printf("No solution found or error occurred.\n");
        }
        
    } else {
        printf("Invalid command or arguments\n");
        return 1;
    }
    
    return 0;
}