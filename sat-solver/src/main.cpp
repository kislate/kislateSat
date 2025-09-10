#include "satSolver.h"
#include "fileoi.h"
#include "sudoku_final.h"
#include "../include/sudoku_player.h"
#include <time.h>
#include <string.h>

// 函数声明
void cnf_mode();
void sudoku_mode();
void cnf_solving_menu(CNF* cnf, CNF* cnf_bak, Assignment* assignment_opt, Assignment* assignment_unopt,
                      SatResult* result_opt, SatResult* result_unopt, double* elapsed_time_opt, double* elapsed_time_unopt,
                      char* input_file, char* output_file);
void wait_for_enter();

int main(int argc, char* argv[])
{
    // 清屏
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    // 主程序循环
    while (1) {
        // 清屏
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        // 显示主菜单
        printf("\n");
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                     Intelligent SAT Solver                    ║\n");
        printf("║               Based on DPLL Algorithm with MOM & JW           ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║                       Main Menu                               ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║    1. CNF File Solving Mode                                   ║\n");
        printf("║      └─ Load DIMACS format CNF file and perform SAT solving   ║\n");
        printf("║                                                               ║\n");
        printf("║    2. Sudoku Game Mode                                        ║\n");
        printf("║      └─ Generate sudoku puzzle and solve using SAT solver     ║\n");
        printf("║                                                               ║\n");
        printf("║    0. Exit Program                                            ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf("Please enter your choice (0-2): ");

        int main_choice;
        scanf("%d", &main_choice);
        while(getchar() != '\n');

        if (main_choice == 0) {
            printf("\n");
            printf("╔═══════════════════════════════════════════════════════════════╗\n");
            printf("║                    Program Terminated                         ║\n");
            printf("║                    Thank you for using!                       ║\n");
            printf("╚═══════════════════════════════════════════════════════════════╝\n");
            break;
        }

        switch(main_choice) {
            case 1:
            {
                cnf_mode();
                break;
            }
            case 2:
            {
                sudoku_mode();
                break;
            }
            default:
            {
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                Invalid choice! Please try again               ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                wait_for_enter();
                break;
            }
        }
    }

    return 0;
}

void cnf_mode() {
    CNF cnf;
    init_dynamic_array(&cnf);
    char input_file[256] = "";
    char output_file[256] = "";
    int cnf_loaded = 0;
    
    // CNF模式状态变量
    Assignment assignment_opt, assignment_unopt;
    CNF cnf_bak;
    SatResult result_opt = UNKNOWN, result_unopt = UNKNOWN;
    double elapsed_time_opt = 0, elapsed_time_unopt = 0;
    int assignments_initialized = 0;

    while (1) {
        // 清屏
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        printf("\n");
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                      CNF Solving Mode                         ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║    1. Load CNF File                                           ║\n");
        if (cnf_loaded) {
            // 截断过长的文件名以保持边框对齐
            char display_filename[41];  // 40字符 + '\0'
            if (strlen(input_file) > 40) {
                strncpy(display_filename, input_file, 37);
                display_filename[37] = '.';
                display_filename[38] = '.';
                display_filename[39] = '.';
                display_filename[40] = '\0';
            } else {
                strcpy(display_filename, input_file);
            }
            printf("║      └─ Current: %-40s     ║\n", display_filename);
        } else {
            printf("║      └─ No file loaded                                        ║\n");
        }
        printf("║                                                               ║\n");
        if (cnf_loaded) {
            printf("║    2. Solving Menu                                            ║\n");
        } else {
            printf("║    2. Solving Menu (Disabled - Load CNF first)                ║\n");
        }
        printf("║    0. Back to Main Menu                                       ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf("Please enter your choice (0-2): ");

        int cnf_choice;
        scanf("%d", &cnf_choice);
        while(getchar() != '\n');

        switch(cnf_choice) {
            case 0:
            {
                // 清理资源并返回
                if (cnf_loaded) {
                    free_dynamic_array(&cnf);
                    if (assignments_initialized) {
                        free_assignment(&assignment_opt);
                        free_assignment(&assignment_unopt);
                        free_dynamic_array(&cnf_bak);
                    }
                }
                return;
            }

            case 1:
            {
                // 加载新的CNF文件
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                        CNF File Loading                       ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                
                // 如果之前已经加载过，先清理
                if (cnf_loaded) {
                    free_dynamic_array(&cnf);
                    if (assignments_initialized) {
                        free_assignment(&assignment_opt);
                        free_assignment(&assignment_unopt);
                        free_dynamic_array(&cnf_bak);
                        assignments_initialized = 0;
                    }
                    init_dynamic_array(&cnf);
                }

                if (load_cnf_interactive(&cnf, input_file, output_file)) {
                    cnf_loaded = 1;
                    // 重置所有求解状态
                    result_opt = UNKNOWN;
                    result_unopt = UNKNOWN;
                    elapsed_time_opt = 0;
                    elapsed_time_unopt = 0;
                    
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                   CNF File Loaded Successfully                ║\n");
                    printf("║                                                               ║\n");
                    printf("║  Variables: %-8d  Clauses: %-8d                       ║\n", cnf.num_variables, cnf.num_clauses);
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                    Failed to load CNF file                    ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                    cnf_loaded = 0;
                }
                break;
            }

            case 2:
            {
                if (!cnf_loaded) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║              Please load a CNF file first!                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                    break;
                }

                // 初始化求解器状态（如果还没有初始化）
                if (!assignments_initialized) {
                    init_assignment(&assignment_opt, cnf.num_variables);
                    init_assignment(&assignment_unopt, cnf.num_variables);
                    init_dynamic_array(&cnf_bak);
                    copy_dynamic_array(&cnf_bak, &cnf);
                    assignments_initialized = 1;
                }

                // 进入求解菜单
                cnf_solving_menu(&cnf, &cnf_bak, &assignment_opt, &assignment_unopt, 
                               &result_opt, &result_unopt, &elapsed_time_opt, &elapsed_time_unopt,
                               input_file, output_file);
                break;
            }
            default:
            {
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                Invalid choice! Please try again               ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                wait_for_enter();
                break;
            }
        }
    }
}

void cnf_solving_menu(CNF* cnf, CNF* cnf_bak, Assignment* assignment_opt, Assignment* assignment_unopt,
                      SatResult* result_opt, SatResult* result_unopt, double* elapsed_time_opt, double* elapsed_time_unopt,
                      char* input_file, char* output_file) {
    while (1) {
        // 清屏
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        printf("\n");
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                       Solving Strategy Menu                   ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║  1. Run Optimized Solver (Occure Max + JW Heuristics)         ║\n");
        if (*result_opt != UNKNOWN) {
            // 确保时间显示格式固定宽度
            if (*elapsed_time_opt < 10000.0) {
                printf("║     └─ Status: Completed (%.0f ms)                              ║\n", *elapsed_time_opt);
            } else {
                printf("║     └─ Status: Completed (%.0f ms)                              ║\n", *elapsed_time_opt);
            }
        } else {
            printf("║     └─ Status: Not executed                                   ║\n");
        }
        printf("║                                                               ║\n");
        printf("║  2. Run Unoptimized Solver (Basic DPLL)                       ║\n");
        if (*result_unopt != UNKNOWN) {
            // 确保时间显示格式固定宽度
            if (*elapsed_time_unopt < 10000.0) {
                printf("║     └─ Status: Completed (%.0f ms)                              ║\n", *elapsed_time_unopt);
            } else {
                printf("║     └─ Status: Completed (%.0f ms)                              ║\n", *elapsed_time_unopt);
            }
        } else {
            printf("║     └─ Status: Not executed                                   ║\n");
        }
        printf("║                                                               ║\n");
        printf("║  3. Run Both Solvers and Compare Performance                  ║\n");
        printf("║  4. Verify Solution                                           ║\n");
        if (*result_opt != UNKNOWN && *result_unopt != UNKNOWN) {
            printf("║  5. Show Optimization Efficiency                              ║\n");
        } else {
            printf("║  5. Show Optimization Efficiency (Disabled - Run both first)  ║\n");
        }
        printf("║  0. Back to CNF Mode Menu                                     ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf("Please enter your choice (0-5): ");

        int solve_choice;
        scanf("%d", &solve_choice);
        while(getchar() != '\n');

        switch(solve_choice) {
            case 0:
            {
                return;
            }

            case 1:
            {
                if (*result_opt == UNKNOWN) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                    Running Optimized Solver...                ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    
                    time_t start_time = clock();
                    *result_opt = solve_sat(cnf, assignment_opt, 0);
                    time_t end_time = clock();
                    *elapsed_time_opt = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000;
                    
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                   Optimized Solver Completed                  ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    print_result(*result_opt, assignment_opt, *elapsed_time_opt);
                    save_result(output_file, *result_opt, assignment_opt, *elapsed_time_opt);
                    wait_for_enter();
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                Optimized solver already executed              ║\n");
                    printf("║                    Showing previous results                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    print_result(*result_opt, assignment_opt, *elapsed_time_opt);
                    wait_for_enter();
                }
                break;
            }

            case 2:
            {
                if (*result_unopt == UNKNOWN) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                   Running Unoptimized Solver...               ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    
                    time_t start_time = clock();
                    *result_unopt = solve_sat(cnf_bak, assignment_unopt, 1);
                    time_t end_time = clock();
                    *elapsed_time_unopt = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000;
                    
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                  Unoptimized Solver Completed                 ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    print_result(*result_unopt, assignment_unopt, *elapsed_time_unopt);
                    wait_for_enter();
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║               Unoptimized solver already executed             ║\n");
                    printf("║                    Showing previous results                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    print_result(*result_unopt, assignment_unopt, *elapsed_time_unopt);
                    wait_for_enter();
                }
                break;
            }

            case 3:
            {
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║               Running Both Solvers for Comparison...          ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                
                if (*result_opt == UNKNOWN) {
                    printf("Running optimized solver...\n");
                    time_t start_time = clock();
                    *result_opt = solve_sat(cnf, assignment_opt, 0);
                    time_t end_time = clock();
                    *elapsed_time_opt = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000;
                }
                
                if (*result_unopt == UNKNOWN) {
                    printf("Running unoptimized solver...\n");
                    time_t start_time = clock();
                    *result_unopt = solve_sat(cnf_bak, assignment_unopt, 1);
                    time_t end_time = clock();
                    *elapsed_time_unopt = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000;
                }
                
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                     Performance Comparison                    ║\n");
                printf("╠═══════════════════════════════════════════════════════════════╣\n");
                // 使用固定宽度格式确保边框对齐
                printf("║  Optimized Solver:   %8.2f ms                          ║\n", *elapsed_time_opt);
                printf("║  Unoptimized Solver: %8.2f ms                          ║\n", *elapsed_time_unopt);
                if (*elapsed_time_unopt > 0) {
                    double efficiency = (*elapsed_time_unopt - *elapsed_time_opt) / *elapsed_time_unopt * 100.0;
                    printf("║  Optimization Efficiency: %6.2f%%                       ║\n", efficiency);
                }
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                wait_for_enter();
                break;
            }

            case 4:
            {
                if (*result_opt == UNKNOWN && *result_unopt == UNKNOWN) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║           Please run at least one solver first!              ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                       Solution Verification                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    verify_result(input_file, output_file);
                    wait_for_enter();
                }
                break;
            }

            case 5:
            {
                if (*result_opt != UNKNOWN && *result_unopt != UNKNOWN && *elapsed_time_unopt > 0) {
                    double efficiency = (*elapsed_time_unopt - *elapsed_time_opt) / *elapsed_time_unopt * 100.0;
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                    Optimization Efficiency Report             ║\n");
                    printf("╠═══════════════════════════════════════════════════════════════╣\n");
                    // 使用固定宽度格式确保边框对齐
                    printf("║  Optimized Time:     %8.2f ms                              ║\n", *elapsed_time_opt);
                    printf("║  Unoptimized Time:   %8.2f ms                              ║\n", *elapsed_time_unopt);
                    printf("║  Time Saved:         %8.2f ms                              ║\n", *elapsed_time_unopt - *elapsed_time_opt);
                    printf("║  Efficiency Gain:    %6.2f%%                                  ║\n", efficiency);
                    if (*elapsed_time_opt > 0) {
                        printf("║  Speed Improvement:  %6.2fx                                  ║\n", *elapsed_time_unopt / *elapsed_time_opt);
                    }
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║            Please run both solvers first to compare           ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                }
                break;
            }

            default:
            {
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                  Invalid choice! Please try again             ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                break;
            }
        }
    }
}

void sudoku_mode() {
    CNF cnf;
    char input_file[256] = "";
    char output_file[256] = "";
    int sudoku_generated = 0;
    
    while (1) {
        // 清屏
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        printf("\n");
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                      Sudoku Game Mode                         ║\n");
        printf("╠═══════════════════════════════════════════════════════════════╣\n");
        printf("║    1. Generate New Sudoku Puzzle                              ║\n");
        if (sudoku_generated) {
            printf("║      └─ Status: Puzzle ready for solving                      ║\n");
        } else {
            printf("║      └─ Status: No puzzle generated                           ║\n");
        }
        printf("║                                                               ║\n");
        if (sudoku_generated) {
            printf("║    2. Solve with SAT Solver                                   ║\n");
            printf("║      └─ Use DPLL algorithm to solve automatically             ║\n");
            printf("║                                                               ║\n");
            printf("║    3. Interactive Player Mode                                 ║\n");
            printf("║      └─ Play sudoku with hints and step-by-step solving       ║\n");
        } else {
            printf("║    2. Solve with SAT Solver (Disabled - Generate first)       ║\n");
            printf("║    3. Interactive Player Mode (Disabled - Generate first)     ║\n");
        }
        printf("║    0. Back to Main Menu                                       ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf("Please enter your choice (0-3): ");

        int sudoku_choice;
        scanf("%d", &sudoku_choice);
        while(getchar() != '\n');

        switch(sudoku_choice) {
            case 0:
                if (sudoku_generated) {
                    free_dynamic_array(&cnf);
                }
                return;

            case 1:
            {
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                    Generating Sudoku Puzzle...                ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                
                // 如果之前已经生成过，先清理
                if (sudoku_generated) {
                    free_dynamic_array(&cnf);
                }
                init_dynamic_array(&cnf);
                
                // 生成数独CNF文件
                generate_sudoku_cnf(input_file, output_file);
                
                // 加载生成的CNF文件
                if (load_cnf_from_file(&cnf, input_file)) {
                    sudoku_generated = 1;
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                  Sudoku Puzzle Generated Successfully         ║\n");
                    printf("║                                                               ║\n");
                    printf("║  Variables: %-8d  Clauses: %-8d                       ║\n", cnf.num_variables, cnf.num_clauses);
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                Failed to generate Sudoku puzzle               ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                    sudoku_generated = 0;
                }
                break;
            }

            case 2:
            {
                if (!sudoku_generated) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║              Please generate a puzzle first!                 ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                    break;
                }

                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                 Solving with SAT Solver...                    ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                
                // 初始化解数组
                Assignment assignment;
                init_assignment(&assignment, cnf.num_variables);
                
                // 求解并且计算时间
                time_t start_time = clock();
                SatResult result = solve_sat(&cnf, &assignment, 0);
                time_t end_time = clock();
                
                double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
                double elapsed_time_ms = elapsed_time * 1000;
                
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                  Sudoku Solving Completed                     ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                
                // 输出结果
                print_result(result, &assignment, elapsed_time_ms);
                save_result(output_file, result, &assignment, elapsed_time_ms);
                
                // 如果求解成功，显示数独解
                if (result == SAT) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                     Sudoku Solution                           ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    printf("    1 2 3   4 5 6   7 8 9\n");
                    printf("  ┌───────┬───────┬───────┐\n");
                    for (int i = 0; i < 9; i++) {
                        printf("%c │ ", 'A' + i);
                        for (int j = 0; j < 9; j++) {
                            // 找到这个位置的数字
                            int num = 0;
                            for (int k = 0; k < 9; k++) {
                                int var = i*81 + j*9 + k + 1;
                                if (assignment.values[var] == TRUE) {
                                    num = k + 1;
                                    break;
                                }
                            }
                            printf("%d ", num);
                            if ((j + 1) % 3 == 0) printf("│ ");
                        }
                        printf("\n");
                        if ((i + 1) % 3 == 0 && i != 8) {
                            printf("  ├───────┼───────┼───────┤\n");
                        }
                    }
                    printf("  └───────┴───────┴───────┘\n");
                }
                
                // 添加暂停让用户查看结果
                wait_for_enter();
                
                // 询问是否验证结果
                printf("\nDo you want to verify the result? (1 for Yes, 0 for No): ");
                int verify_choice;
                scanf("%d", &verify_choice);
                while(getchar() != '\n');
                
                if (verify_choice == 1) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                       Solution Verification                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    verify_result(input_file, output_file);
                    wait_for_enter();
                }
                
                // 询问是否进入交互式求解模式
                printf("\nDo you want to enter interactive solving mode? (1 for Yes, 0 for No): ");
                int play_choice;
                scanf("%d", &play_choice);
                while(getchar() != '\n');
                
                if (play_choice == 1) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║                   Interactive Sudoku Player                   ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    sudoku_player_interface(&assignment);
                }
                
                // 释放资源
                free_assignment(&assignment);
                break;
            }
            case 3:
            {
                if (!sudoku_generated) {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║              Please generate a puzzle first!                 ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                    break;
                }

                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                   Interactive Player Mode                     ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                
                // 先用SAT求解器获取完整解（作为答案）
                Assignment solution_assignment;
                init_assignment(&solution_assignment, cnf.num_variables);
                
                SatResult result = solve_sat(&cnf, &solution_assignment, 0);
                
                if (result == SAT) {
                    // 调用交互式数独玩家界面，传递CNF和解
                    sudoku_player_interface_with_cnf(&cnf, &solution_assignment, input_file);
                } else {
                    printf("\n");
                    printf("╔═══════════════════════════════════════════════════════════════╗\n");
                    printf("║              Puzzle has no solution! Cannot play.             ║\n");
                    printf("╚═══════════════════════════════════════════════════════════════╝\n");
                    wait_for_enter();
                }
                
                // 释放资源
                free_assignment(&solution_assignment);
                break;
            }
            default:
            {
                printf("\n");
                printf("╔═══════════════════════════════════════════════════════════════╗\n");
                printf("║                Invalid choice! Please try again               ║\n");
                printf("╚═══════════════════════════════════════════════════════════════╝\n");
                wait_for_enter();
                break;
            }
        }
    }
}

void wait_for_enter() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                     Press Enter to continue...                ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    while (getchar() != '\n');
}