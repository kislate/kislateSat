#include "sudoku.h"
#include "sat_solver.h"
#include "fileop.h"

// 初始化数独网格
void init_sudoku_grid(SudokuGrid* sudoku) {
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            sudoku->grid[i][j] = 0;
        }
    }
    sudoku->filled_cells = 0;
}

// 检查在指定位置放置数字是否有效
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
    
    return 1;
}

// 回溯法求解数独
int solve_sudoku_backtrack(SudokuGrid* sudoku) {
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int col = 0; col < SUDOKU_SIZE; col++) {
            if (sudoku->grid[row][col] == 0) {
                for (int num = 1; num <= 9; num++) {
                    if (is_valid_placement(sudoku, row, col, num)) {
                        sudoku->grid[row][col] = num;
                        sudoku->filled_cells++;
                        
                        if (solve_sudoku_backtrack(sudoku)) {
                            return 1;
                        }
                        
                        // 回溯
                        sudoku->grid[row][col] = 0;
                        sudoku->filled_cells--;
                    }
                }
                return 0;  // 无解
            }
        }
    }
    return 1;  // 已解决
}

// 生成完整的数独解
void generate_full_sudoku(SudokuGrid* sudoku) {
    init_sudoku_grid(sudoku);
    
    // 随机填充一些初始数字以增加随机性
    srand(time(NULL));
    
    // 在对角线宫格中填充一些数字
    for (int box = 0; box < 3; box++) {
        int start_row = box * 3;
        int start_col = box * 3;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int num = (i * 3 + j + box * 3) % 9 + 1;
                if (is_valid_placement(sudoku, start_row + i, start_col + j, num)) {
                    sudoku->grid[start_row + i][start_col + j] = num;
                    sudoku->filled_cells++;
                }
            }
        }
    }
    
    // 用回溯法完成剩余部分
    solve_sudoku_backtrack(sudoku);
}

// 挖洞法创建数独题目
void create_puzzle_by_digging(SudokuGrid* sudoku, int holes_count) {
    if (holes_count > SUDOKU_CELLS) holes_count = SUDOKU_CELLS - 17;  // 至少保留17个格子
    
    int dug_holes = 0;
    while (dug_holes < holes_count) {
        int row = rand() % SUDOKU_SIZE;
        int col = rand() % SUDOKU_SIZE;
        
        if (sudoku->grid[row][col] != 0) {
            // 临时挖掉这个格子
            int original_value = sudoku->grid[row][col];
            sudoku->grid[row][col] = 0;
            sudoku->filled_cells--;
            
            // 检查是否仍有唯一解（简化版检查）
            SudokuGrid test_sudoku = *sudoku;
            if (solve_sudoku_backtrack(&test_sudoku)) {
                // 有解，保持挖掉的状态
                dug_holes++;
            } else {
                // 无解，恢复原值
                sudoku->grid[row][col] = original_value;
                sudoku->filled_cells++;
            }
        }
    }
}

// 打印数独网格
void print_sudoku_grid(const SudokuGrid* sudoku) {
    printf("Sudoku Grid (%d filled cells):\n", sudoku->filled_cells);
    printf("   1 2 3   4 5 6   7 8 9\n");
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        if (i % 3 == 0 && i > 0) {
            printf("  -------+-------+-------\n");
        }
        printf("%c ", 'A' + i);
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (j % 3 == 0 && j > 0) printf("| ");
            if (sudoku->grid[i][j] == 0) {
                printf("%% ");  // 用百分号表示空格
            } else {
                printf("%d ", sudoku->grid[i][j]);
            }
        }
        printf("\n");
    }
}

// 保存为.ss格式（百分号数独）
void save_sudoku_ss_format(const SudokuGrid* sudoku, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create file %s\n", filename);
        return;
    }
    
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (sudoku->grid[i][j] == 0) {
                fprintf(file, "%%");
            } else {
                fprintf(file, "%d", sudoku->grid[i][j]);
            }
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("Sudoku puzzle saved to: %s\n", filename);
}

// 获取变量编号：第row行、第col列、数字num对应的变量编号
int get_variable_number(int row, int col, int num) {
    return row * SUDOKU_SIZE * SUDOKU_SIZE + col * SUDOKU_SIZE + num;
}

// 添加数独约束到CNF
void add_sudoku_constraints(CNF* cnf) {
    // 1. 每个格子必须有且只有一个数字
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int col = 0; col < SUDOKU_SIZE; col++) {
            // 至少有一个数字
            Clause at_least_one;
            init_literal_array(&at_least_one.literals);
            for (int num = 1; num <= SUDOKU_SIZE; num++) {
                push_literal(&at_least_one.literals, get_variable_number(row, col, num));
            }
            push_clause(&cnf->clauses, &at_least_one);
            free_literal_array(&at_least_one.literals);
            
            // 最多有一个数字（两两互斥）
            for (int num1 = 1; num1 <= SUDOKU_SIZE; num1++) {
                for (int num2 = num1 + 1; num2 <= SUDOKU_SIZE; num2++) {
                    Clause at_most_one;
                    init_literal_array(&at_most_one.literals);
                    push_literal(&at_most_one.literals, -get_variable_number(row, col, num1));
                    push_literal(&at_most_one.literals, -get_variable_number(row, col, num2));
                    push_clause(&cnf->clauses, &at_most_one);
                    free_literal_array(&at_most_one.literals);
                }
            }
        }
    }
    
    // 2. 每行每个数字只出现一次
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int num = 1; num <= SUDOKU_SIZE; num++) {
            // 每行至少有一个num
            Clause row_constraint;
            init_literal_array(&row_constraint.literals);
            for (int col = 0; col < SUDOKU_SIZE; col++) {
                push_literal(&row_constraint.literals, get_variable_number(row, col, num));
            }
            push_clause(&cnf->clauses, &row_constraint);
            free_literal_array(&row_constraint.literals);
            
            // 每行最多有一个num
            for (int col1 = 0; col1 < SUDOKU_SIZE; col1++) {
                for (int col2 = col1 + 1; col2 < SUDOKU_SIZE; col2++) {
                    Clause row_unique;
                    init_literal_array(&row_unique.literals);
                    push_literal(&row_unique.literals, -get_variable_number(row, col1, num));
                    push_literal(&row_unique.literals, -get_variable_number(row, col2, num));
                    push_clause(&cnf->clauses, &row_unique);
                    free_literal_array(&row_unique.literals);
                }
            }
        }
    }
    
    // 3. 每列每个数字只出现一次
    for (int col = 0; col < SUDOKU_SIZE; col++) {
        for (int num = 1; num <= SUDOKU_SIZE; num++) {
            // 每列至少有一个num
            Clause col_constraint;
            init_literal_array(&col_constraint.literals);
            for (int row = 0; row < SUDOKU_SIZE; row++) {
                push_literal(&col_constraint.literals, get_variable_number(row, col, num));
            }
            push_clause(&cnf->clauses, &col_constraint);
            free_literal_array(&col_constraint.literals);
            
            // 每列最多有一个num
            for (int row1 = 0; row1 < SUDOKU_SIZE; row1++) {
                for (int row2 = row1 + 1; row2 < SUDOKU_SIZE; row2++) {
                    Clause col_unique;
                    init_literal_array(&col_unique.literals);
                    push_literal(&col_unique.literals, -get_variable_number(row1, col, num));
                    push_literal(&col_unique.literals, -get_variable_number(row2, col, num));
                    push_clause(&cnf->clauses, &col_unique);
                    free_literal_array(&col_unique.literals);
                }
            }
        }
    }
    
    // 4. 每个3x3宫格每个数字只出现一次
    for (int box_row = 0; box_row < 3; box_row++) {
        for (int box_col = 0; box_col < 3; box_col++) {
            for (int num = 1; num <= SUDOKU_SIZE; num++) {
                // 每个宫格至少有一个num
                Clause box_constraint;
                init_literal_array(&box_constraint.literals);
                for (int r = 0; r < 3; r++) {
                    for (int c = 0; c < 3; c++) {
                        int row = box_row * 3 + r;
                        int col = box_col * 3 + c;
                        push_literal(&box_constraint.literals, get_variable_number(row, col, num));
                    }
                }
                push_clause(&cnf->clauses, &box_constraint);
                free_literal_array(&box_constraint.literals);
                
                // 每个宫格最多有一个num
                for (int pos1 = 0; pos1 < 9; pos1++) {
                    for (int pos2 = pos1 + 1; pos2 < 9; pos2++) {
                        int row1 = box_row * 3 + pos1 / 3;
                        int col1 = box_col * 3 + pos1 % 3;
                        int row2 = box_row * 3 + pos2 / 3;
                        int col2 = box_col * 3 + pos2 % 3;
                        
                        Clause box_unique;
                        init_literal_array(&box_unique.literals);
                        push_literal(&box_unique.literals, -get_variable_number(row1, col1, num));
                        push_literal(&box_unique.literals, -get_variable_number(row2, col2, num));
                        push_clause(&cnf->clauses, &box_unique);
                        free_literal_array(&box_unique.literals);
                    }
                }
            }
        }
    }
}

// 将数独转换为CNF
void sudoku_to_cnf(const SudokuGrid* sudoku, CNF* cnf) {
    // 初始化CNF
    init_cnf(cnf);
    cnf->num_variables = SUDOKU_SIZE * SUDOKU_SIZE * SUDOKU_SIZE;  // 9*9*9 = 729个变量
    
    // 添加基本数独约束
    add_sudoku_constraints(cnf);
    
    // 添加已知数字的约束
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int col = 0; col < SUDOKU_SIZE; col++) {
            if (sudoku->grid[row][col] != 0) {
                int num = sudoku->grid[row][col];
                Clause known_cell;
                init_literal_array(&known_cell.literals);
                push_literal(&known_cell.literals, get_variable_number(row, col, num));
                push_clause(&cnf->clauses, &known_cell);
                free_literal_array(&known_cell.literals);
            }
        }
    }
    
    cnf->num_clauses = cnf->clauses.size;
    printf("Sudoku converted to CNF: %d variables, %d clauses\n", cnf->num_variables, cnf->num_clauses);
}

// 保存数独为CNF格式
void save_sudoku_cnf_format(const SudokuGrid* sudoku, const char* filename) {
    CNF cnf;
    sudoku_to_cnf(sudoku, &cnf);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create CNF file %s\n", filename);
        free_cnf(&cnf);
        return;
    }
    
    // 写入CNF头部
    fprintf(file, "c Sudoku puzzle converted to CNF\n");
    fprintf(file, "c Generated by SAT Solver\n");
    fprintf(file, "p cnf %d %d\n", cnf.num_variables, cnf.num_clauses);
    
    // 写入子句
    for (int i = 0; i < cnf.clauses.size; i++) {
        Clause* clause = &cnf.clauses.data[i];
        for (int j = 0; j < clause->literals.size; j++) {
            fprintf(file, "%d ", clause->literals.data[j]);
        }
        fprintf(file, "0\n");
    }
    
    fclose(file);
    free_cnf(&cnf);
    printf("CNF file saved to: %s\n", filename);
}

// 生成并求解数独的主函数
int generate_and_solve_sudoku() {
    printf("=== Sudoku Generator and Solver ===\n");
    
    // 创建sudoku_cnf文件夹
#ifdef _WIN32
    system("mkdir sudoku_cnf 2>nul");
#else
    system("mkdir -p sudoku_cnf");
#endif
    
    printf("Generating sudoku puzzle...\n");
    
    SudokuGrid sudoku;
    
    // 生成完整数独
    generate_full_sudoku(&sudoku);
    printf("Full sudoku generated!\n");
    
    // 询问用户挖洞数目
    printf("Enter number of holes to dig (17-64, recommended 25-55): ");
    int holes;
    scanf("%d", &holes);
    while (getchar() != '\n');  // 清除输入缓冲区
    
    // 限制挖洞数目范围
    if (holes < 17) {
        printf("Too few holes, using minimum 17\n");
        holes = 17;
    } else if (holes > 64) {
        printf("Too many holes, using maximum 64\n");
        holes = 64;
    }
    
    // 创建题目（挖洞）
    create_puzzle_by_digging(&sudoku, holes);
    
    printf("Puzzle created with %d empty cells:\n", SUDOKU_CELLS - sudoku.filled_cells);
    print_sudoku_grid(&sudoku);
    
    // 保存为.ss格式
    char ss_filename[256];
    snprintf(ss_filename, sizeof(ss_filename), "sudoku_cnf/puzzle_%lld.ss", (long long)time(NULL));
    save_sudoku_ss_format(&sudoku, ss_filename);
    
    // 转换为CNF并保存
    char cnf_filename[256];
    snprintf(cnf_filename, sizeof(cnf_filename), "sudoku_cnf/puzzle_%lld.cnf", (long long)time(NULL));
    save_sudoku_cnf_format(&sudoku, cnf_filename);
    
    // 求解CNF
    printf("\nSolving sudoku using SAT solver...\n");
    CNF cnf;
    sudoku_to_cnf(&sudoku, &cnf);
    
    Assignment assignment;
    init_assignment(&assignment, cnf.num_variables);
    
    clock_t start_time = clock();
    SatResult result = dpll_solve(&cnf, &assignment);
    clock_t end_time = clock();
    
    double elapsed_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000;
    
    printf("Sudoku solving completed!\n");
    printf("Result: %s\n", (result == SAT) ? "SAT (Sudoku solved!)" : "UNSAT (No solution)");
    printf("Solving time: %.2f ms\n", elapsed_time_ms);
    
    if (result == SAT) {
        printf("\nSudoku solution:\n");
        // 解析SAT结果回数独格式
        SudokuGrid solution;
        init_sudoku_grid(&solution);
        
        for (int row = 0; row < SUDOKU_SIZE; row++) {
            for (int col = 0; col < SUDOKU_SIZE; col++) {
                for (int num = 1; num <= SUDOKU_SIZE; num++) {
                    int var = get_variable_number(row, col, num);
                    if (var <= assignment.size && assignment.values[var] == TRUE) {
                        solution.grid[row][col] = num;
                        solution.filled_cells++;
                        break;
                    }
                }
            }
        }
        print_sudoku_grid(&solution);
    }
    
    // 清理内存
    free_assignment(&assignment);
    free_cnf(&cnf);
    
    return (result == SAT) ? 1 : 0;
}
