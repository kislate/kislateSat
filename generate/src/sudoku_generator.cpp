#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sudoku_generator.h"

// 检查在位置(row,col)放置num是否合法
int isValidPlace(int grid[9][9], int row, int col, int num) {
    // 检查行
    for (int x = 0; x < 9; x++) {
        if (grid[row][x] == num) return 0;
    }
    
    // 检查列
    for (int x = 0; x < 9; x++) {
        if (grid[x][col] == num) return 0;
    }
    
    // 检查3x3盒子
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i + startRow][j + startCol] == num) return 0;
        }
    }
    
    return 1;
}

// 随机打乱数组
void shuffleArray(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// 生成完整数独解（回溯算法）
int solveSudoku(int grid[9][9]) {
    int row, col;
    int isEmpty = 0;
    
    // 找到第一个空格子
    for (row = 0; row < 9; row++) {
        for (col = 0; col < 9; col++) {
            if (grid[row][col] == 0) {
                isEmpty = 1;
                break;
            }
        }
        if (isEmpty) break;
    }
    
    // 如果没有空格子，说明完成了
    if (!isEmpty) return 1;
    
    // 创建1-9的随机数组
    int numbers[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffleArray(numbers, 9);
    
    // 尝试填入每个数字
    for (int i = 0; i < 9; i++) {
        if (isValidPlace(grid, row, col, numbers[i])) {
            grid[row][col] = numbers[i];
            if (solveSudoku(grid)) {
                return 1;
            }
            grid[row][col] = 0;  // 回溯
        }
    }
    
    return 0;
}

// 生成完整的数独解
void generateCompleteSudoku(int grid[9][9]) {
    // 初始化为空
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            grid[i][j] = 0;
        }
    }
    
    // 生成解
    solveSudoku(grid);
}

// 挖洞函数
void digHoles(int grid[9][9], int holes) {
    int positions[81][2];
    int count = 0;
    
    // 创建所有位置的数组
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            positions[count][0] = i;
            positions[count][1] = j;
            count++;
        }
    }
    
    // 随机打乱位置
    for (int i = 80; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp0 = positions[i][0];
        int temp1 = positions[i][1];
        positions[i][0] = positions[j][0];
        positions[i][1] = positions[j][1];
        positions[j][0] = temp0;
        positions[j][1] = temp1;
    }
    
    // 挖洞
    for (int i = 0; i < holes && i < 81; i++) {
        int row = positions[i][0];
        int col = positions[i][1];
        grid[row][col] = 0;
    }
}

// 打印数独
void printSudoku(int grid[9][9]) {
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0 && i != 0) {
            printf("------+-------+------\n");
        }
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0 && j != 0) {
                printf("| ");
            }
            if (grid[i][j] == 0) {
                printf(". ");
            } else {
                printf("%d ", grid[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

// 将数独转换为变量编号
int sudokuToVar(int row, int col, int num) {
    return (row * 81) + (col * 9) + num;
}

// 生成格约束子句
void generateCellConstraints(FILE* file, int* clauseCount) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            // 至少填一个数字
            for (int k = 1; k <= 9; k++) {
                fprintf(file, "%d ", sudokuToVar(i, j, k));
            }
            fprintf(file, "0\n");
            (*clauseCount)++;
            
            // 不能同时填两个数字
            for (int k1 = 1; k1 <= 9; k1++) {
                for (int k2 = k1 + 1; k2 <= 9; k2++) {
                    fprintf(file, "-%d -%d 0\n", sudokuToVar(i, j, k1), sudokuToVar(i, j, k2));
                    (*clauseCount)++;
                }
            }
        }
    }
}

// 生成行约束子句
void generateRowConstraints(FILE* file, int* clauseCount) {
    for (int i = 0; i < 9; i++) {
        for (int k = 1; k <= 9; k++) {
            // 每行必须包含数字k
            for (int j = 0; j < 9; j++) {
                fprintf(file, "%d ", sudokuToVar(i, j, k));
            }
            fprintf(file, "0\n");
            (*clauseCount)++;
            
            // 每行不能有两个相同的数字
            for (int j1 = 0; j1 < 9; j1++) {
                for (int j2 = j1 + 1; j2 < 9; j2++) {
                    fprintf(file, "-%d -%d 0\n", sudokuToVar(i, j1, k), sudokuToVar(i, j2, k));
                    (*clauseCount)++;
                }
            }
        }
    }
}

// 生成列约束子句
void generateColumnConstraints(FILE* file, int* clauseCount) {
    for (int j = 0; j < 9; j++) {
        for (int k = 1; k <= 9; k++) {
            // 每列必须包含数字k
            for (int i = 0; i < 9; i++) {
                fprintf(file, "%d ", sudokuToVar(i, j, k));
            }
            fprintf(file, "0\n");
            (*clauseCount)++;
            
            // 每列不能有两个相同的数字
            for (int i1 = 0; i1 < 9; i1++) {
                for (int i2 = i1 + 1; i2 < 9; i2++) {
                    fprintf(file, "-%d -%d 0\n", sudokuToVar(i1, j, k), sudokuToVar(i2, j, k));
                    (*clauseCount)++;
                }
            }
        }
    }
}

// 生成3x3盒子约束子句
void generateBoxConstraints(FILE* file, int* clauseCount) {
    for (int boxRow = 0; boxRow < 3; boxRow++) {
        for (int boxCol = 0; boxCol < 3; boxCol++) {
            for (int k = 1; k <= 9; k++) {
                // 每个盒子必须包含数字k
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        int row = boxRow * 3 + i;
                        int col = boxCol * 3 + j;
                        fprintf(file, "%d ", sudokuToVar(row, col, k));
                    }
                }
                fprintf(file, "0\n");
                (*clauseCount)++;
                
                // 每个盒子不能有两个相同的数字
                int cells[9][2];
                int cellCount = 0;
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        cells[cellCount][0] = boxRow * 3 + i;
                        cells[cellCount][1] = boxCol * 3 + j;
                        cellCount++;
                    }
                }
                
                for (int c1 = 0; c1 < 9; c1++) {
                    for (int c2 = c1 + 1; c2 < 9; c2++) {
                        fprintf(file, "-%d -%d 0\n", 
                                sudokuToVar(cells[c1][0], cells[c1][1], k),
                                sudokuToVar(cells[c2][0], cells[c2][1], k));
                        (*clauseCount)++;
                    }
                }
            }
        }
    }
}

// 生成提示数约束
void generateHintConstraints(FILE* file, int grid[9][9], int* clauseCount) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (grid[i][j] != 0) {
                fprintf(file, "%d 0\n", sudokuToVar(i, j, grid[i][j]));
                (*clauseCount)++;
            }
        }
    }
}

// 保存数独为CNF文件
void saveSudokuToCNF(int grid[9][9], const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Cannot create CNF file: %s\n", filename);
        return;
    }
    
    // 计算子句数量
    int clauseCount = 0;
    // 格约束: 81格 × (1 + 36) = 81 × 37 = 2997
    clauseCount += 81 * 37;
    // 行约束: 9行 × 9数字 × (1 + 36) = 2997
    clauseCount += 2997;
    // 列约束: 9列 × 9数字 × (1 + 36) = 2997
    clauseCount += 2997;
    // 盒子约束: 9盒子 × 9数字 × (1 + 36) = 2997
    clauseCount += 2997;
    // 提示数约束: 最多81个
    int hintCount = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (grid[i][j] != 0) hintCount++;
        }
    }
    clauseCount += hintCount;
    
    fprintf(file, "c Generated Sudoku puzzle\n");
    fprintf(file, "p cnf 729 %d\n", clauseCount);
    
    // 生成各种约束
    generateCellConstraints(file, &clauseCount);
    generateRowConstraints(file, &clauseCount);
    generateColumnConstraints(file, &clauseCount);
    generateBoxConstraints(file, &clauseCount);
    generateHintConstraints(file, grid, &clauseCount);
    
    fclose(file);
    printf("Sudoku saved to CNF file: %s with %d clauses\n", filename, clauseCount);
}