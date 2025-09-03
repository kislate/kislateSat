#include "fileop.h"
#include <string.h>  // 显式包含，确保strrchr可用
// =========== 加载cnf文件 ===========

int load_cnf_from_file(CNF* cnf, const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) 
    {
        // fprintf(stderr, "无法打开文件: %s\n", filename);
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }
    
    char line[1024];
    int variables_found = 0;
    
    while (fgets(line, sizeof(line), file))
    {
        // 跳过注释行
        if (line[0] == 'c') continue;
        
        // cnf 的问题描述
        if (line[0] == 'p') {
            char format[10];
            // 读取到三个才是正确
            if (sscanf(line, "p %s %d %d", format, &cnf->num_variables, &cnf->num_clauses) != 3)
            {
                // fprintf(stderr, "无效的问题行格式\n");
                fprintf(stderr, "Invalid problem line format\n");
                fclose(file);
                return 0;
            }
            if (strcmp(format, "cnf") != 0)
            {
                // fprintf(stderr, "不支持的格式: %s\n", format);
                fprintf(stderr, "Unsupported format: %s\n", format);
                fclose(file);
                return 0;
            }
            variables_found = 1;
            continue;
        }
        
        // 如果还没有读取到变量数，跳过
        if (!variables_found) continue;
        
        // 读取子句
        Clause clause;
        init_literal_array(&clause.literals);
        // char temp = line[0];
        char *token = strtok(line, " \t\n");
        while (token)
        {
            int literal = atoi(token);  // string to int
            if (literal == 0) break; // 结束标志
            push_literal(&clause.literals, literal);
            token = strtok(NULL, " \t\n"); // 接着之前的继续
        }
        
        if (clause.literals.size > 0)
        {
            push_clause(&cnf->clauses, &clause);
        }
        free_literal_array(&clause.literals);
    }
    
    fclose(file);
    
    // 调试信息（通过宏控制）
    #ifdef DEBUG
    printf("Successfully Read CNF File:\n");
    printf("  Number of Variables: %d\n", cnf->num_variables);
    printf("  Number of Clauses: %d (Actual Read: %d)\n", cnf->num_clauses, cnf->clauses.size);
    #endif
    
    return 1;
}

// 交互式CNF文件加载，带用户输入
int load_cnf_interactive(CNF* cnf, char* filename_out)
{
    printf("=== CNF File Loading ===\n");
    printf("Please select loading method:\n");
    printf("1. Select from test folder\n");
    printf("2. Enter path manually\n");
    printf("Enter your choice (1/2): ");
    
    int choice;
    scanf("%d", &choice);
    
    // 清除输入缓冲区
    while (getchar() != '\n');
    
    char input_file[256];
    
    if (choice == 1) {
        // 列出测试文件，紧凑显示
        printf("\nAvailable test files:\n");
        printf("1.1.cnf          2.2.cnf          3.3.cnf\n");
        printf("4.4(unsat).cnf   5.5.cnf          6.6.cnf\n");
        printf("7.7(unsat).cnf   8.8(unsat).cnf   9.9(unsat).cnf\n");
        printf("10.10.cnf        11.11(unsat).cnf 12.12.cnf\n");
        printf("Enter file number (1-12): ");
        
        int file_num;
        scanf("%d", &file_num);
        while (getchar() != '\n');
        
        switch (file_num) {
            case 1: strcpy(input_file, "test/1.cnf"); break;
            case 2: strcpy(input_file, "test/2.cnf"); break;
            case 3: strcpy(input_file, "test/3.cnf"); break;
            case 4: strcpy(input_file, "test/4u.cnf"); break;
            case 5: strcpy(input_file, "test/5.cnf"); break;
            case 6: strcpy(input_file, "test/6.cnf"); break;
            case 7: strcpy(input_file, "test/7u.cnf"); break;
            case 8: strcpy(input_file, "test/8u.cnf"); break;
            case 9: strcpy(input_file, "test/9u.cnf"); break;
            case 10: strcpy(input_file, "test/10.cnf"); break;
            case 11: strcpy(input_file, "test/11u.cnf"); break;
            case 12: strcpy(input_file, "test/12.cnf"); break;
            default:
                printf("Invalid selection, using test/1.cnf\n");
                strcpy(input_file, "test/1.cnf");
                break;
        }
    } else if (choice == 2) {
        printf("Enter CNF file path: ");
        fgets(input_file, sizeof(input_file), stdin);
        
        // 移除换行符
        int len = strlen(input_file);
        if (len > 0 && input_file[len-1] == '\n') {
            input_file[len-1] = '\0';
        }
    } else {
        printf("Invalid choice, using test/1.cnf\n");
        strcpy(input_file, "test/1.cnf");
    }
    
    printf("Loading CNF file: %s\n", input_file);
    
    // 复制文件名用于输出
    if (filename_out) {
        strcpy(filename_out, input_file);
    }
    
    // 检查文件是否存在
    FILE* test_file = fopen(input_file, "r");
    if (!test_file) {
        printf("Error: Cannot open input file: %s\n", input_file);
        return 0;
    }
    fclose(test_file);
    
    // 加载CNF文件
    if (!load_cnf_from_file(cnf, input_file)) {
        printf("Error: Cannot load CNF file\n");
        return 0;
    }
    
    printf("Successfully loaded CNF file!\n");
    printf("  Variables: %d\n", cnf->num_variables);
    printf("  Clauses: %d\n", cnf->num_clauses);
    
    return 1;
}

// =========== 输出结果实现 ===========

void save_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        // printf("无法创建输出文件: %s\n", filename);
        printf("Unable to Create Output File: %s\n", filename);
        return;
    }
    
    if (result == SAT) {
        fprintf(file, "s 1\n");
        fprintf(file, "v ");
        for (int i = 1; i <= assignment->size; i++) {
            if (assignment->values[i] == TRUE) {
                fprintf(file, "%d ", i);
            } else if (assignment->values[i] == FALSE) {
                fprintf(file, "%d ", -i);
            }
        }
        fprintf(file, "\n");
    } else {
        fprintf(file, "s 0\n");
    }
    
    fprintf(file, "t %.0f\n", elapsed_time_ms);
    
    fclose(file);
}

void save_and_print_result(const char* input_file, SatResult result, const Assignment* assignment, double elapsed_time_ms)
{
    // 生成输出文件名
    char output_file[256];
    const char* last_slash = strrchr(input_file, '/');
    const char* last_backslash = strrchr(input_file, '\\');
    const char* filename = input_file;

    // 这里的目的是找到文件名部分, (为了在ubuntu和windows下都能用, 还有防止我的手误)
    if (last_slash && last_backslash)
    {
        filename = (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
    }
    else if (last_slash)
    {
        filename = last_slash + 1;
    } 
    else if (last_backslash)
    {
        filename = last_backslash + 1;
    }

    // 移除掉扩展名
    char base_name[256];
    strcpy(base_name, filename);
    char* dot = strrchr(base_name, '.');
    if (dot) *dot = '\0'; // 直接断掉

    // 格式化字符串
    snprintf(output_file, sizeof(output_file), "res/%s.res", base_name);
    // 保证我能在ubuntu中正常使用, 但是windows.h可能要删掉
#ifdef _WIN32
    system("mkdir res 2>nul");
#else
    system("mkdir -p res");
#endif
    save_result(output_file, result, assignment, elapsed_time_ms);
    // 保存结果
    // printf("结果已保存到: %s\n", output_file);
    printf("Result saved to: %s\n", output_file);
    
    // 如果是SAT，显示部分赋值
    if (result == SAT)
    {
        // printf("满足的赋值 (前20个变量): ");
        printf("Satisfying Assignment (First 20 Variables): ");
        for (int i = 1; i <= assignment->size && i <= 20; i++)
        {
            if (assignment->values[i] == TRUE) printf("%d ", i);
            else printf("%d ", -i);
        }
        printf("\n");
    }
    
    // 调用验证功能
    verify_result(input_file, output_file);
}

// 验证结果函数
int verify_result(const char* cnf_file, const char* res_file)
{
    char command[512];
    char verify_choice;
    char verifier_choice;
    
    // 询问是否要验证结果
    // printf("\n是否要验证结果的正确性？(y/n): ");
    printf("\nDo you want to verify the result? (y/n): ");
    verify_choice = getchar();
    
    // 清除输入缓冲区
    while (getchar() != '\n');
    
    if (verify_choice != 'y' && verify_choice != 'Y') {
        return 1; // 用户选择不验证，返回成功
    }
    
    // 选择验证程序
    // printf("请选择验证程序:\n");
    // printf("1. verify.exe\n");
    // printf("2. verify5000.exe\n");
    // printf("请输入选择 (1/2): ");
    printf("Please select the verifier program (1/2): ");
    printf("1. verify.exe\n");
    printf("2. verify5000.exe\n");
    printf("What you choice (1/2): ");
    verifier_choice = getchar();
    
    // 清除输入缓冲区
    while (getchar() != '\n');
    
    const char* verifier;
    if (verifier_choice == '1') {
        verifier = "verify\\verify.exe";
    } else if (verifier_choice == '2') {
        verifier = "verify\\verify5000.exe";
    } else {
        printf("Invalid choice, using default verifier verify.exe\n");
        verifier = "verify\\verify.exe";
    }
    
    // 构建命令
    snprintf(command, sizeof(command), "%s \"%s\" \"%s\"", verifier, cnf_file, res_file);
    
    // printf("正在验证结果...\n");
    // printf("执行命令: %s\n", command);
    printf("Verifying result...\n");
    printf("Executing command: %s\n", command);

    // 执行验证命令
    int result = system(command);
    
    if (result == 0) {
        printf("Verification Passed: Result is Correct!\n");
        return 1;
    } else {
        printf("Verification Failed: Result may be Incorrect!\n");
        return 0;
    }
}

