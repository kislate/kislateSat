#include "../include/sudoku_player.h"

void init_sudoku_player(SudokuPlayer* player) {
    // 初始化所有数组为0
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            player->grid[i][j] = EMPTY_CELL;
            player->original[i][j] = EMPTY_CELL;
            player->solution[i][j] = EMPTY_CELL;
            player->player_filled[i][j] = 0;
        }
    }
}

void free_sudoku_player(SudokuPlayer* player) {
}

int load_sudoku_from_solution(SudokuPlayer* player, Assignment* assignment) {
    // 从SAT求解器的结果中提取完整的数独解
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int found = 0;
            for (int k = 0; k < 9; k++) {
                int var = i * 81 + j * 9 + k + 1;
                if (assignment->values[var] == TRUE) {
                    player->solution[i][j] = k + 1;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("Error: Invalid solution from SAT solver\n");
                return 0;
            }
        }
    }
    
    // 创建一个部分填充的谜题作为原始题目
    // 随机移除一些数字作为玩家需要填写的位置
    srand((unsigned int)time(NULL));
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            // 保留约30%的数字作为提示
            if (rand() % 100 < 30) {
                player->original[i][j] = player->solution[i][j];
                player->grid[i][j] = player->solution[i][j];
            } else {
                player->original[i][j] = EMPTY_CELL;
                player->grid[i][j] = EMPTY_CELL;
            }
            player->player_filled[i][j] = 0;
        }
    }
    
    return 1;
}

int load_sudoku_from_cnf(SudokuPlayer* player, CNF* cnf, Assignment* solution) {
    // 先从解中提取完整答案
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int found = 0;
            for (int k = 0; k < 9; k++) {
                int var = i * 81 + j * 9 + k + 1;
                if (solution->values[var] == TRUE) {
                    player->solution[i][j] = k + 1;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("Error: Invalid solution from SAT solver\n");
                return 0;
            }
        }
    }
    
    // 初始化原始网格为空
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            player->original[i][j] = EMPTY_CELL;
            player->grid[i][j] = EMPTY_CELL;
            player->player_filled[i][j] = 0;
        }
    }
    
    // 从CNF的单元子句中提取原始题目的提示数字
    for (int i = 0; i < cnf->num_clauses; i++) {
        if (cnf->data[i].size == 1) {  // 单元子句表示确定的数字
            int literal = cnf->data[i].data[0];
            if (literal > 0) {  // 正文字
                int var = literal - 1;  // 转换为0-based
                int row = var / 81;
                int col = (var % 81) / 9;
                int num = (var % 9) + 1;
                
                if (row >= 0 && row < 9 && col >= 0 && col < 9 && num >= 1 && num <= 9) {
                    player->original[row][col] = num;
                    player->grid[row][col] = num;
                }
            }
        }
    }
    
    return 1;
}

void display_sudoku_grid(SudokuPlayer* player, int show_solution) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    if (show_solution) {
        printf("║                        Sudoku Solution                        ║\n");
    } else {
        printf("║                        Sudoku Puzzle                          ║\n");
    }
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    printf("    1 2 3   4 5 6   7 8 9\n");
    printf("  ┌───────┬───────┬───────┐\n");
    
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        printf("%c │ ", 'A' + i);
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            int value_to_show;
            if (show_solution) {
                value_to_show = player->solution[i][j];
            } else {
                value_to_show = player->grid[i][j];
            }
            
            if (value_to_show == EMPTY_CELL) {
                printf(". ");
            } else {
                // 区分原始数字和玩家填写的数字
                if (show_solution || player->original[i][j] != EMPTY_CELL) {
                    printf("%d ", value_to_show);  // 原始数字
                } else if (player->player_filled[i][j]) {
                    printf("%d ", value_to_show);  // 玩家填写的数字
                } else {
                    printf("%d ", value_to_show);
                }
            }
            if ((j + 1) % 3 == 0) printf("│ ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0 && i != 8) {
            printf("  ├───────┼───────┼───────┤\n");
        }
    }
    printf("  └───────┴───────┴───────┘\n");
    
    if (!show_solution) {
        printf("\nLegend: '.' = empty, numbers = filled cells\n");
        printf("Original clues and player entries are shown the same way\n");
    }
}

void display_sudoku_with_errors(SudokuPlayer* player) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                      Error Check Results                      ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    int error_count = 0;
    printf("    1 2 3   4 5 6   7 8 9\n");
    printf("  ┌───────┬───────┬───────┐\n");
    
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        printf("%c │ ", 'A' + i);
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (player->grid[i][j] == EMPTY_CELL) {
                printf(". ");
            } else if (player->grid[i][j] == player->solution[i][j]) {
                printf("%d ", player->grid[i][j]);  // 正确的数字
            } else {
                printf("X ");  // 错误的数字
                error_count++;
            }
            if ((j + 1) % 3 == 0) printf("│ ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0 && i != 8) {
            printf("  ├───────┼───────┼───────┤\n");
        }
    }
    printf("  └───────┴───────┴───────┘\n");
    
    if (error_count == 0) {
        printf("\n✓ All filled cells are correct!\n");
    } else {
        printf("\n✗ Found %d error(s). Errors are marked with 'X'.\n", error_count);
    }
}

int player_fill_cell(SudokuPlayer* player, int row, int col, int value) {
    // 检查坐标有效性
    if (row < 0 || row >= SUDOKU_SIZE || col < 0 || col >= SUDOKU_SIZE) {
        printf("Error: Invalid position. Row must be A-I, column must be 1-9.\n");
        return 0;
    }
    
    // 检查是否是原始题目的位置（不能修改）
    if (player->original[row][col] != EMPTY_CELL) {
        printf("Error: Cannot modify original clue at position %c%d.\n", 'A' + row, col + 1);
        return 0;
    }
    
    // 检查数值有效性
    if (value < 0 || value > 9) {
        printf("Error: Value must be 0-9 (0 to clear cell).\n");
        return 0;
    }
    
    // 填写数字
    if (value == 0) {
        player->grid[row][col] = EMPTY_CELL;
        player->player_filled[row][col] = 0;
        printf("Cell %c%d cleared.\n", 'A' + row, col + 1);
    } else {
        player->grid[row][col] = value;
        player->player_filled[row][col] = 1;
        printf("Cell %c%d set to %d.\n", 'A' + row, col + 1, value);
    }
    
    return 1;
}

void give_random_hint(SudokuPlayer* player) {
    // 找到所有空白的位置
    int empty_positions[SUDOKU_SIZE * SUDOKU_SIZE][2];
    int empty_count = 0;
    
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (player->grid[i][j] == EMPTY_CELL) {
                empty_positions[empty_count][0] = i;
                empty_positions[empty_count][1] = j;
                empty_count++;
            }
        }
    }
    
    if (empty_count == 0) {
        printf("No empty cells available for hints!\n");
        return;
    }
    
    // 随机选择一个空白位置
    int random_index = rand() % empty_count;
    int row = empty_positions[random_index][0];
    int col = empty_positions[random_index][1];
    
    // 给出提示
    player->grid[row][col] = player->solution[row][col];
    player->player_filled[row][col] = 0;  // 标记为提示，不是玩家填写
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                         Random Hint                           ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  Position %c%d has been filled with %d                     ║\n", 'A' + row, col + 1, player->solution[row][col]);
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
}

void give_specific_hint(SudokuPlayer* player, int row, int col) {
    // 检查坐标有效性
    if (row < 0 || row >= SUDOKU_SIZE || col < 0 || col >= SUDOKU_SIZE) {
        printf("Error: Invalid position.\n");
        return;
    }
    
    // 检查位置是否已经填写
    if (player->grid[row][col] != EMPTY_CELL) {
        printf("Position %c%d is already filled with %d.\n", 'A' + row, col + 1, player->grid[row][col]);
        return;
    }
    
    // 给出特定位置的提示
    player->grid[row][col] = player->solution[row][col];
    player->player_filled[row][col] = 0;  // 标记为提示
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                        Specific Hint                          ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  Position %c%d has been filled with %d                         ║\n", 'A' + row, col + 1, player->solution[row][col]);
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
}

int check_sudoku_completion(SudokuPlayer* player) {
    // 检查是否所有位置都已填写
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (player->grid[i][j] == EMPTY_CELL) {
                return 0;  // 还有空白位置
            }
        }
    }
    
    // 检查是否所有填写都正确
    for (int i = 0; i < SUDOKU_SIZE; i++) {
        for (int j = 0; j < SUDOKU_SIZE; j++) {
            if (player->grid[i][j] != player->solution[i][j]) {
                return -1;  // 完成但有错误
            }
        }
    }
    
    return 1;  // 完成且正确
}

int validate_sudoku_input(int row, int col, int value) {
    return (row >= 0 && row < SUDOKU_SIZE && 
            col >= 0 && col < SUDOKU_SIZE && 
            value >= 0 && value <= 9);
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void wait_for_player_enter() {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n');
}

void sudoku_player_interface(Assignment* assignment) {
    SudokuPlayer player;
    init_sudoku_player(&player);
    
    // 加载数独题目
    if (!load_sudoku_from_solution(&player, assignment)) {
        printf("Failed to load sudoku puzzle.\n");
        return;
    }
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Interactive Sudoku Player                  ║\n");
    printf("║                                                               ║\n");
    printf("║  Welcome to the interactive sudoku player!                    ║\n");
    printf("║  Try to complete the puzzle by filling in the missing numbers ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    while (1) {
        // 清屏
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        // 显示当前数独状态
        display_sudoku_grid(&player, 0);
        
        // 显示菜单
        printf("\n");
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                     Sudoku Player Menu                        ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║  1. Fill a cell (e.g., A1 5)                                  ║\n");
        printf("║  2. Get random hint                                           ║\n");
        printf("║  3. Get specific hint (e.g., A1)                              ║\n");
        printf("║  4. Show solution temporarily                                 ║\n");
        printf("║  5. Check for errors                                          ║\n");
        printf("║  6. Submit final answer                                       ║\n");
        printf("║  0. Return to sudoku menu                                     ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf("Please enter your choice (0-6): ");
        
        int choice;
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch (choice) {
            case 0:
                free_sudoku_player(&player);
                return;
                
            case 1: {
                printf("Enter position and value (e.g., 'A1 5' or 'B3 0' to clear): ");
                char pos[10];
                int value;
                if (scanf("%s %d", pos, &value) == 2) {
                    if (strlen(pos) >= 2) {
                        int row = pos[0] - 'A';
                        int col = pos[1] - '1';
                        player_fill_cell(&player, row, col, value);
                    } else {
                        printf("Invalid input format. Use format like 'A1 5'.\n");
                    }
                } else {
                    printf("Invalid input format. Use format like 'A1 5'.\n");
                }
                clear_input_buffer();
                wait_for_player_enter();
                break;
            }
            
            case 2:
                give_random_hint(&player);
                wait_for_player_enter();
                break;
                
            case 3: {
                printf("Enter position for hint (e.g., 'A1'): ");
                char pos[10];
                if (scanf("%s", pos) == 1 && strlen(pos) >= 2) {
                    int row = pos[0] - 'A';
                    int col = pos[1] - '1';
                    give_specific_hint(&player, row, col);
                } else {
                    printf("Invalid input format. Use format like 'A1'.\n");
                }
                clear_input_buffer();
                wait_for_player_enter();
                break;
            }
            
            case 4:
                display_sudoku_grid(&player, 1);  // 显示答案
                wait_for_player_enter();
                break;
                
            case 5:
                display_sudoku_with_errors(&player);
                wait_for_player_enter();
                break;
                
            case 6: {
                int completion_status = check_sudoku_completion(&player);
                if (completion_status == 1) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                     🎉 Congratulations! 🎉                   ║\n");
                    printf("║                                                               ║\n");
                    printf("║         You have successfully completed the sudoku!           ║\n");
                    printf("║                    All answers are correct!                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                } else if (completion_status == -1) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                      Puzzle Complete but...                   ║\n");
                    printf("║                                                               ║\n");
                    printf("║         Some answers are incorrect. Please try again!         ║\n");
                    printf("║              Use 'Check for errors' to see mistakes           ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                      Puzzle Incomplete                        ║\n");
                    printf("║                                                               ║\n");
                    printf("║            There are still empty cells to fill!               ║\n");
                    printf("║                  Please continue playing.                     ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                }
                wait_for_player_enter();
                break;
            }
            
            default:
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                  Invalid choice! Please try again             ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                wait_for_player_enter();
                break;
        }
    }
}

void sudoku_player_interface_with_cnf(CNF* cnf, Assignment* solution, const char* cnf_filename) {
    SudokuPlayer player;
    init_sudoku_player(&player);
    
    // 使用CNF和解来加载数独谜题
    if (!load_sudoku_from_cnf(&player, cnf, solution)) {
        printf("Failed to load sudoku puzzle from CNF.\n");
        return;
    }
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Interactive Sudoku Player                  ║\n");
    printf("║                                                               ║\n");
    printf("║  Welcome to the interactive sudoku player!                    ║\n");
    printf("║  Try to complete the puzzle by filling in the missing numbers ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    while (1) {
        // 清屏
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        // 显示当前数独状态
        display_sudoku_grid(&player, 0);
        
        // 显示菜单
        printf("\n");
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                     Sudoku Player Menu                        ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║  1. Fill a cell (e.g., A1 5)                                  ║\n");
        printf("║  2. Get random hint                                           ║\n");
        printf("║  3. Get specific hint (e.g., A1)                              ║\n");
        printf("║  4. Show solution temporarily                                 ║\n");
        printf("║  5. Check for errors                                          ║\n");
        printf("║  6. Submit final answer                                       ║\n");
        printf("║  0. Return to sudoku menu                                     ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf("Please enter your choice (0-6): ");
        
        int choice;
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch (choice) {
            case 0:
                free_sudoku_player(&player);
                return;
                
            case 1: {
                printf("Enter position and value (e.g., 'A1 5' or 'B3 0' to clear): ");
                char pos[10];
                int value;
                if (scanf("%s %d", pos, &value) == 2) {
                    if (strlen(pos) >= 2) {
                        int row = pos[0] - 'A';
                        int col = pos[1] - '1';
                        player_fill_cell(&player, row, col, value);
                    } else {
                        printf("Invalid input format. Use format like 'A1 5'.\n");
                    }
                } else {
                    printf("Invalid input format. Use format like 'A1 5'.\n");
                }
                clear_input_buffer();
                wait_for_player_enter();
                break;
            }
            
            case 2:
                give_random_hint(&player);
                wait_for_player_enter();
                break;
                
            case 3: {
                printf("Enter position for hint (e.g., 'A1'): ");
                char pos[10];
                if (scanf("%s", pos) == 1 && strlen(pos) >= 2) {
                    int row = pos[0] - 'A';
                    int col = pos[1] - '1';
                    give_specific_hint(&player, row, col);
                } else {
                    printf("Invalid input format. Use format like 'A1'.\n");
                }
                clear_input_buffer();
                wait_for_player_enter();
                break;
            }
            
            case 4:
                display_sudoku_grid(&player, 1);  // 显示答案
                wait_for_player_enter();
                break;
                
            case 5:
                display_sudoku_with_errors(&player);
                wait_for_player_enter();
                break;
                
            case 6: {
                int completion_status = check_sudoku_completion(&player);
                if (completion_status == 1) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                   🎉 CONGRATULATIONS! 🎉                      ║\n");
                    printf("║                                                               ║\n");
                    printf("║           You have successfully solved the puzzle!            ║\n");
                    printf("║                   Well done, Sudoku Master!                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                } else if (completion_status == -1) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                      Puzzle Complete But...                   ║\n");
                    printf("║                                                               ║\n");
                    printf("║              Some answers are incorrect! 😞                   ║\n");
                    printf("║          Please check your work and try again.                ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                      Puzzle Incomplete                        ║\n");
                    printf("║                                                               ║\n");
                    printf("║            There are still empty cells to fill!               ║\n");
                    printf("║                  Please continue playing.                     ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                }
                wait_for_player_enter();
                break;
            }
            
            default:
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                  Invalid choice! Please try again             ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                wait_for_player_enter();
                break;
        }
    }
}
