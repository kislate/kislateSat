#include "satSolver.h"
#include "fileoi.h"
#include "sudoku_final.h"

#include <time.h>

int main(int argc, char* argv[])
{
    printf("=== SAT SOLVER ===\n");
    printf("Please select mode:\n");
    printf("1. Load CNF file and solve\n");
    printf("2. Generate and solve Sudoku puzzle\n");
    printf("Enter your choice (1/2): ");

    int mode_choice;
    int mode;
    scanf("%d", &mode_choice);
    mode = mode_choice;
    while(getchar() != '\n'); // 清除输入缓冲区
    char input_file[256];
    char output_file[256];

    // 初始化 CNF
    CNF cnf;
    init_dynamic_array(&cnf);

    // sat结果
    SatResult result = UNSAT;

    switch(mode_choice)
    {
        case 1:
        {
            // 获取cnf文件
            if (!load_cnf_interactive(&cnf, input_file, output_file)) {
                    free_dynamic_array(&cnf);
                    return 1;
            }

            // 初始化解数组
            Assignment assignment;
            init_assignment(&assignment, cnf.num_variables);

            // 求解并且计算时间
            time_t start_time = clock();
            result = solve_sat(&cnf, &assignment);
            time_t end_time = clock();

            // 计算耗时
            double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            double elapsed_time_ms = elapsed_time * 1000;

            // 输出结果
            print_result(result, &assignment, elapsed_time_ms);
            save_result(output_file, result, &assignment, elapsed_time_ms);

            // 询问是否验证结果
            int verify_choice;
            printf("Do you want to verify the result? (1 for Yes, 0 for No): ");
            scanf("%d", &verify_choice);
            if (verify_choice == 1) verify_result(input_file, output_file);

            // 释放资源
            free_dynamic_array(&cnf);
            free_assignment(&assignment);
            break;
        }

        case 2:
        {
            // 数独生成和求解模式
            printf("\n=== Sudoku Generation and Solving ===\n");
            
            // 生成数独CNF文件
            generate_sudoku_cnf(input_file, output_file);
            
            // 加载生成的CNF文件
            if (!load_cnf_from_file(&cnf, input_file)) {
                printf("Failed to load generated CNF file\n");
                return 1;
            }
            
            // 初始化解数组
            Assignment assignment;
            init_assignment(&assignment, cnf.num_variables);
            
            // 求解并且计算时间
            printf("\nSolving sudoku...\n");
            time_t start_time = clock();
            result = solve_sat(&cnf, &assignment);
            time_t end_time = clock();
            
            double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            double elapsed_time_ms = elapsed_time * 1000;
            
            // 输出结果（使用和case 1相同的格式）
            print_result(result, &assignment, elapsed_time_ms);
            save_result(output_file, result, &assignment, elapsed_time_ms);
            
            // 如果求解成功，显示数独解
            if (result == SAT) {
                printf("\n=== Sudoku Solution ===\n");
                printf("  1 2 3   4 5 6   7 8 9\n");
                for (int i = 0; i < 9; i++) {
                    printf("%c ", 'A' + i);
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
                        if ((j + 1) % 3 == 0) printf(" ");
                    }
                    printf("\n");
                    if ((i + 1) % 3 == 0) printf("\n");
                }
            }
            
            // 询问是否验证结果
            int verify_choice;
            printf("Do you want to verify the result? (1 for Yes, 0 for No): ");
            scanf("%d", &verify_choice);
            if (verify_choice == 1) verify_result(input_file, output_file);
            
            // 释放资源
            free_dynamic_array(&cnf);
            free_assignment(&assignment);
            break;
        }
    }
    printf("=== Program End ===\n");
    return 0;
}
