#include "sat_solver.h"
#include <time.h>
#include <windows.h>

// =============================================================================
// 调试模式控制
// 取消注释下面这行以启用详细的调试输出（包括求解过程中的实时状态）
// =============================================================================
#define DEBUG

int main(int argc, char* argv[]) {
    // 防止中文乱码:
    SetConsoleOutputCP(65001);
    printf("=== SAT 求解器 ===\n");
    
    if (argc != 2) {
        printf("用法: %s <cnf_file>\n", argv[0]);
        return 1;
    }
    
    char* input_file = argv[1];
    
    // 检查输入文件是否存在
    FILE* test_file = fopen(input_file, "r");
    if (!test_file) {
        printf("错误：无法打开输入文件 %s\n", input_file);
        return 1;
    }
    fclose(test_file);
    
    printf("输入文件: %s\n", input_file);
    
    // 初始化CNF
    CNF cnf;
    init_cnf(&cnf);
    
    // 加载CNF文件
    if (!load_cnf_from_file(&cnf, input_file)) {
        printf("错误：无法加载CNF文件\n");
        free_cnf(&cnf);
        return 1;
    }
    
    // 初始化赋值
    Assignment assignment;
    init_assignment(&assignment, cnf.num_variables);
    
    // 重置求解状态计数器
    extern int dpll_call_count, unit_propagation_count, backtrack_count;
    extern time_t last_output_time;
    dpll_call_count = 0;
    unit_propagation_count = 0;
    backtrack_count = 0;
    last_output_time = time(NULL);
    
    printf("开始求解...\n");
    clock_t start_time = clock();
    
    // 求解
    SatResult result = dpll_solve(&cnf, &assignment);
    
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    double elapsed_time_ms = elapsed_time * 1000;
    
    // 输出结果
    printf("求解完成！\n");
    printf("结果: %s\n", (result == SAT) ? "可满足 (SAT)" : 
                      (result == UNSAT) ? "不可满足 (UNSAT)" : "未知");
    printf("求解时间: %.0f ms\n", elapsed_time_ms);
    
#ifdef DEBUG
    printf("统计信息: DPLL调用%d次, 单元传播%d次, 回溯%d次\n", 
           dpll_call_count, unit_propagation_count, backtrack_count);
#endif
    
    // 生成输出文件名
    char output_file[256];
    char* last_slash = strrchr(input_file, '/');
    char* last_backslash = strrchr(input_file, '\\');
    char* filename = input_file;
    
    if (last_slash && last_backslash) {
        filename = (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
    } else if (last_slash) {
        filename = last_slash + 1;
    } else if (last_backslash) {
        filename = last_backslash + 1;
    }
    
    // 移除扩展名
    char base_name[256];
    strcpy(base_name, filename);
    char* dot = strrchr(base_name, '.');
    if (dot) {
        *dot = '\0';
    }
    
    snprintf(output_file, sizeof(output_file), "res/%s.res", base_name);
    
    // 确保res目录存在
    system("mkdir res 2>nul");
    
    // 保存结果
    save_result(output_file, result, &assignment, elapsed_time_ms);
    printf("结果已保存到: %s\n", output_file);
    
    // 如果是SAT，显示部分赋值
    if (result == SAT) {
        printf("满足的赋值 (前20个变量): ");
        for (int i = 1; i <= assignment.size && i <= 20; i++) {
            if (assignment.values[i] == TRUE) {
                printf("%d ", i);
            } else {
                printf("%d ", -i);
            }
        }
        if (assignment.size > 20) {
            printf("...");
        }
        printf("\n");
    }
    
    // 清理内存
    free_assignment(&assignment);
    free_cnf(&cnf);
    
    printf("程序结束\n");
    return 0;
}
