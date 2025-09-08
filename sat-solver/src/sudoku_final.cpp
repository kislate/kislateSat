#include "sudoku_final.h"

#define SUDOKU_SIZE 9
#define SUDOKU_CELLS 81

// 数独网格结构
typedef struct {
    int grid[SUDOKU_SIZE][SUDOKU_SIZE];
    int filled_cells;
} SudokuGrid;

// 初始化数独网格
void init_sudoku_grid(SudokuGrid* sudoku) {
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            sudoku->grid[i][j] = 0;
        }
    }
    sudoku->filled_cells = 0;
}

// 检查在指定位置放置数字是否有效（百分号数独版本）
int is_valid_placement(const SudokuGrid* sudoku, int row, int col, int num) {
    // 检查行
    for (int j = 0; j < SUDOKU_SIZE; j++) {
        if (sudoku->grid[row][j] == num) return 0;
    }
    
    // 检查列
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        if (sudoku->grid[i][col] == num) return 0;
    }
    
    // 检查3x3宫格
    int box_row = (row / 3) * 3;
    int box_col = (col / 3) * 3;
    for (int i = box_row; i < box_row + 3; i++) {
        for (int j = box_col; j < box_col + 3; j++) {
            if (sudoku->grid[i][j] == num) return 0;
        }
    }
    
    // 检查副对角线
    if (row + col == SUDOKU_SIZE - 1) {
        for (int i = 0; i < SUDOKU_SIZE; i++) {
            if (sudoku->grid[i][SUDOKU_SIZE - 1 - i] == num) return 0;
        }
    }
    
    // 检查百分号上面那个点
    if (row >= 1 && row <= 3 && col >= 1 && col <= 3) {
        for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= 3; j++) {
                if (sudoku->grid[i][j] == num) return 0;
            }
        }
    }
    
    // 检查百分号下面那个点
    if (row >= 5 && row <= 7 && col >= 5 && col <= 7) {
        for (int i = 5; i <= 7; i++) {
            for (int j = 5; j <= 7; j++) {
                if (sudoku->grid[i][j] == num) return 0;
            }
        }
    }
    
    return 1;
}

// 随机打乱数组
void shuffle_array(int* array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// 递归填充完整数独
int fill_sudoku_recursive(SudokuGrid* sudoku, int pos) {
    if (pos == SUDOKU_CELLS) return 1;
    
    int row = pos / SUDOKU_SIZE;
    int col = pos % SUDOKU_SIZE;
    
    // 创建1-9的随机序列
    int nums[SUDOKU_SIZE];
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        nums[i] = i + 1;
    }
    shuffle_array(nums, SUDOKU_SIZE);
    
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        if (is_valid_placement(sudoku, row, col, nums[i])) {
            sudoku->grid[row][col] = nums[i];
            sudoku->filled_cells++;
            if (fill_sudoku_recursive(sudoku, pos + 1)) {
                return 1;
            }
            sudoku->grid[row][col] = 0;
            sudoku->filled_cells--;
        }
    }
    return 0;
}

// 生成完整的数独解, 封装
void generate_full_sudoku(SudokuGrid* sudoku) {
    init_sudoku_grid(sudoku);
    srand(time(NULL));
    fill_sudoku_recursive(sudoku, 0);
}

// 解数独的计数器，用于唯一解判定
static int solution_count = 0;

// 检查唯一解的求解函数
void solve_for_uniqueness(SudokuGrid* sudoku, int pos) {
    if (pos == SUDOKU_CELLS) {
        solution_count++;
        return;
    }
    if (solution_count > 1) return; // 两个即判错
    
    int row = pos / SUDOKU_SIZE;
    int col = pos % SUDOKU_SIZE;
    
    if (sudoku->grid[row][col] != 0) {
        solve_for_uniqueness(sudoku, pos + 1);
        return;
    }
    
    for (int val = 1; val <= SUDOKU_SIZE; val++) {
        if (is_valid_placement(sudoku, row, col, val)) {
            sudoku->grid[row][col] = val;
            solve_for_uniqueness(sudoku, pos + 1);
            sudoku->grid[row][col] = 0;
            if (solution_count > 1) return; // 剪枝
        }
    }
}

// 挖洞法创建数独题目
void create_puzzle_by_digging(SudokuGrid* sudoku, int holes_count) {
    if (holes_count > SUDOKU_CELLS - 17) holes_count = SUDOKU_CELLS - 17;  // 至少保留17个格子
    
    int attempts = holes_count;
    while (attempts > 0) {
        int row = rand() % SUDOKU_SIZE;
        int col = rand() % SUDOKU_SIZE;
        
        if (sudoku->grid[row][col] == 0) continue; // 已经是空格
        
        // 备份原值
        int backup = sudoku->grid[row][col];
        sudoku->grid[row][col] = 0;
        sudoku->filled_cells--;
        
        // 检查唯一解
        solution_count = 0;
        solve_for_uniqueness(sudoku, 0);
        
        if (solution_count != 1) {
            // 不是唯一解，恢复原值
            sudoku->grid[row][col] = backup;
            sudoku->filled_cells++;
        } else {
            // 是唯一解，成功挖掉一个洞
            attempts--;
        }
    }
}

// 打印数独网格（百分号格式）
void print_sudoku_grid(const SudokuGrid* sudoku) {
    printf("Sudoku Grid (%d filled cells):\n", sudoku->filled_cells);
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        if (i % 3 == 0 && i > 0) {
            printf("  -------+-------+-------\n");
        }
        printf("%c ", 'A' + i);
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (j % 3 == 0 && j > 0) printf("| ");
            if (sudoku->grid[i][j] == 0) {
                printf("%% ");  // 百分号表示空格
            } else {
                printf("%d ", sudoku->grid[i][j]);
            }
        }
        printf("\n");
    }
}

// 获取变量编号：第row行、第col列、数字num对应的变量编号
int get_variable_number(int row, int col, int num) {
    return row * SUDOKU_SIZE * SUDOKU_SIZE + col * SUDOKU_SIZE + num;
}

// =========== 生成数独CNF文件 ===========
void generate_sudoku_cnf(char* input_file, char* output_file) {
    
    printf("\n=== Sudoku Generator ===\n");
    printf("Enter number of holes (17-64, recommended 25-55): ");
    int holes;
    scanf("%d", &holes);
    while(getchar() != '\n');
    
    // 限制范围
    if (holes < 17) {
        printf("Too few holes, using minimum 17\n");
        holes = 17;
    } else if (holes > 64) {
        printf("Too many holes, using maximum 64\n");
        holes = 64;
    }
    
    // 生成数独
    printf("Generating sudoku puzzle...\n");
    SudokuGrid sudoku;
    generate_full_sudoku(&sudoku);
    printf("Full sudoku generated!\n");
    
    create_puzzle_by_digging(&sudoku, holes);
    
    printf("Puzzle created with %d empty cells:\n", SUDOKU_CELLS - sudoku.filled_cells);
    print_sudoku_grid(&sudoku);
    
    // 生成文件名
    time_t timestamp = time(NULL);
    sprintf(input_file, "sudoku_cnf/puzzle_%d.cnf", (int)timestamp);
    sprintf(output_file, "sudoku_res/puzzle_%d.res", (int)timestamp);
    
    // 创建CNF文件
    FILE* file = fopen(input_file, "w");
    if (!file) {
        printf("Failed to create CNF file: %s\n", input_file);
        return;
    }
    
    // 计算子句数量 
    int estimated_clauses = 81 + 2916*4 + 36 + 36 + 36 + sudoku.filled_cells; // 约12000+
    fprintf(file, "c Percent Sudoku CNF with special constraints\n");
    fprintf(file, "p cnf 729 %d\n", estimated_clauses); // 估计值, 反正不影响我的结果
    
    // get_variable_number(row, col, num) = row*81 + col*9 + num (num是1-9)
    
    // 1. 每个格子必须有且只有一个数字
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int col = 0; col < SUDOKU_SIZE; col++) {
            // 至少有一个数字
            for (int num = 1; num <= SUDOKU_SIZE; num++) {
                fprintf(file, "%d ", get_variable_number(row, col, num));
            }
            fprintf(file, "0\n");
            
            // 最多有一个数字（两两互斥）
            for (int num1 = 1; num1 <= SUDOKU_SIZE; num1++) {
                for (int num2 = num1 + 1; num2 <= SUDOKU_SIZE; num2++) {
                    fprintf(file, "-%d -%d 0\n", 
                           get_variable_number(row, col, num1),
                           get_variable_number(row, col, num2));
                }
            }
        }
    }
    
    // 2. 每行每个数字只出现一次
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int num = 1; num <= SUDOKU_SIZE; num++) {
            // 每行最多有一个num
            for (int col1 = 0; col1 < SUDOKU_SIZE; col1++) {
                for (int col2 = col1 + 1; col2 < SUDOKU_SIZE; col2++) {
                    fprintf(file, "-%d -%d 0\n",
                           get_variable_number(row, col1, num),
                           get_variable_number(row, col2, num));
                }
            }
        }
    }
    
    // 3. 每列每个数字只出现一次
    for (int col = 0; col < SUDOKU_SIZE; col++) {
        for (int num = 1; num <= SUDOKU_SIZE; num++) {
            // 每列最多有一个num
            for (int row1 = 0; row1 < SUDOKU_SIZE; row1++) {
                for (int row2 = row1 + 1; row2 < SUDOKU_SIZE; row2++) {
                    fprintf(file, "-%d -%d 0\n",
                           get_variable_number(row1, col, num),
                           get_variable_number(row2, col, num));
                }
            }
        }
    }
    
    // 4. 每个3x3宫格每个数字只出现一次
    for (int box_row = 0; box_row < 3; box_row++) {
        for (int box_col = 0; box_col < 3; box_col++) {
            for (int num = 1; num <= SUDOKU_SIZE; num++) {
                // 每个宫格最多有一个num
                for (int pos1 = 0; pos1 < 9; pos1++) {
                    for (int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                        int row1 = box_row * 3 + pos1 / 3;
                        int col1 = box_col * 3 + pos1 % 3;
                        int row2 = box_row * 3 + pos2 / 3;
                        int col2 = box_col * 3 + pos2 % 3;
                        
                        fprintf(file, "-%d -%d 0\n",
                               get_variable_number(row1, col1, num),
                               get_variable_number(row2, col2, num));
                    }
                }
            }
        }
    }
    
    // 5. 百分号数独特殊约束：副对角线每个数字只出现一次
    for (int num = 1; num <= SUDOKU_SIZE; num++) {
        // 副对角线最多有一个num
        for (int pos1 = 0; pos1 < SUDOKU_SIZE; pos1++) {
            for (int pos2 = pos1 + 1; pos2 < SUDOKU_SIZE; pos2++) {
                fprintf(file, "-%d -%d 0\n",
                       get_variable_number(pos1, SUDOKU_SIZE - 1 - pos1, num),
                       get_variable_number(pos2, SUDOKU_SIZE - 1 - pos2, num));
            }
        }
    }
    
    // 6. 百分号数独特殊约束：区域(1,1)到(3,3)每个数字只出现一次
    for (int num = 1; num <= SUDOKU_SIZE; num++) {
        // 区域最多有一个num
        for (int pos1 = 0; pos1 < 9; pos1++) {
            for (int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                int row1 = 1 + pos1 / 3;
                int col1 = 1 + pos1 % 3;
                int row2 = 1 + pos2 / 3;
                int col2 = 1 + pos2 % 3;
                
                fprintf(file, "-%d -%d 0\n",
                       get_variable_number(row1, col1, num),
                       get_variable_number(row2, col2, num));
            }
        }
    }
    
    // 7. 百分号数独特殊约束：区域(5,5)到(7,7)每个数字只出现一次
    for (int num = 1; num <= SUDOKU_SIZE; num++) {
        // 区域最多有一个num
        for (int pos1 = 0; pos1 < 9; pos1++) {
            for (int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                int row1 = 5 + pos1 / 3;
                int col1 = 5 + pos1 % 3;
                int row2 = 5 + pos2 / 3;
                int col2 = 5 + pos2 % 3;
                
                fprintf(file, "-%d -%d 0\n",
                       get_variable_number(row1, col1, num),
                       get_variable_number(row2, col2, num));
            }
        }
    }
    
    // 8. 已知数字的约束
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (sudoku.grid[i][j] != 0) {
                fprintf(file, "%d 0\n", get_variable_number(i, j, sudoku.grid[i][j]));
            }
        }
    }
    
    fclose(file);
    
    printf("\nSudoku CNF file saved: %s\n", input_file);
    printf("Variables: 729, Known digits: %d\n", sudoku.filled_cells);
}
