#include "sat_solver.h"
#include "fileop.h"

#include "sat_solver.h"
#include "fileop.h"
#include "sudoku.h"

int main(int argc, char* argv[]) {
    printf("=== SAT SOLVER ===\n");
    printf("Please select mode:\n");
    printf("1. Generate and solve Sudoku puzzle\n");
    printf("2. Load CNF file and solve\n");
    printf("Enter your choice (1/2): ");
    
    int mode_choice;
    scanf("%d", &mode_choice);
    while (getchar() != '\n');  // 清除输入缓冲区
    
    if (mode_choice == 1) {
        // 数独生成和求解模式
        int result = generate_and_solve_sudoku();
        printf("\nSudoku generation and solving %s\n", result ? "completed successfully" : "failed");
    } else if (mode_choice == 2) {
        // 原有的CNF求解功能
        // Initialize CNF
        CNF cnf;
        init_cnf(&cnf);
        
        // 输入cnf文件
        char input_file[256];
        if (!load_cnf_interactive(&cnf, input_file)) {
            free_cnf(&cnf);
            return 1;
        }
        
        // 初始化解数组
        Assignment assignment;
        init_assignment(&assignment, cnf.num_variables);
        
        // 重置统计数据
        extern int dpll_call_count, unit_propagation_count, backtrack_count;
        extern time_t last_output_time;
        dpll_call_count = 0;
        unit_propagation_count = 0;
        backtrack_count = 0;
        last_output_time = time(NULL);
        
        printf("\nStart Solving...\n");
        clock_t start_time = clock();
        
        // 求解
        SatResult result = dpll_solve(&cnf, &assignment);
        
        clock_t end_time = clock();
        double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        double elapsed_time_ms = elapsed_time * 1000;
        
        // 输出结果
        printf("Solving Completed!\n");
        printf("Result: %s\n", (result == SAT) ? "Satisfiable (SAT)" : 
                          (result == UNSAT) ? "Unsatisfiable (UNSAT)" : "Unknown");
        printf("Solving Time: %.0f ms\n", elapsed_time_ms);
        
    #ifdef DEBUG
        printf("Statistics: DPLL Calls: %d, Unit Propagations: %d, Backtracks: %d\n", 
               dpll_call_count, unit_propagation_count, backtrack_count);
    #endif

        // 保存并打印结果
        save_and_print_result(input_file, result, &assignment, elapsed_time_ms);
        
        // 清空
        free_assignment(&assignment);
        free_cnf(&cnf);
    } else {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    printf("Program Ended\n");
    return 0;
}
