#include "fileoi.h"

// 从文件加载CNF
int load_cnf_from_file(CNF* cnf, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return 0;
    }
    
    char line[1024];
    int variables_found = 0;
    
    // 初始化CNF
    init_dynamic_array(cnf);
    
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释行
        if (line[0] == 'c') continue;
        // 处理问题描述行
        if (line[0] == 'p') {
            char format[10];
            if (sscanf(line, "p %s %d %d", format, &cnf->num_variables, &cnf->num_clauses) != 3) {
                printf("Invalid problem line format\n");
                fclose(file);
                return 0;
            }
            if (strcmp(format, "cnf") != 0) {
                printf("Unsupported format: %s\n", format);
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
        init_dynamic_array(&clause);
        
        char* token = strtok(line, " \t\n");
        while (token) {
            int literal = atoi(token);
            if (literal == 0) break; // 子句结束标志
            push_dynamic_array(&clause, &literal);
            token = strtok(NULL, " \t\n");
        }
        
        if (clause.size > 0) {
            push_dynamic_array(cnf, &clause);
        }
    }
    
    fclose(file);
    
    printf("Successfully loaded CNF file:\n");
    printf("  Variables: %d\n", cnf->num_variables);
    printf("  Clauses: %d\n", cnf->size);
    
    return 1;
}

// 交互式加载CNF文件
int load_cnf_interactive(CNF* cnf, char* filename_out, char *output_filename) {
    // 从test文件夹中读取1-12.cnf还是自定义
    printf("Which mode do you want?\n");
    printf("1. Load from test files (1-12.cnf)\n");
    printf("2. Load from custom file path\n");
    printf("Enter your choice (1/2): ");
    int mode_choice;
    scanf("%d", &mode_choice);
    while (getchar() != '\n');  // 清除输入缓冲区
    if (mode_choice == 1) {
        int file_number;
        printf("Enter file number (1-12): ");
        scanf("%d", &file_number);
        while (getchar() != '\n');  // 清除输入缓冲区
        if (file_number < 1 || file_number > 12) {
            printf("Invalid file number. Please enter a number between 1 and 12.\n");
            return 0;
        }
        // sprintf(filename_out, "test/%d.cnf", file_number);
        // 4, 7, 8, 9, 11 不满足
        if (file_number == 4 || file_number == 7 || file_number == 8 || file_number == 9 || file_number == 11) {
            sprintf(filename_out, "./test/%du.cnf", file_number);
        }
        else{
            sprintf(filename_out, "./test/%d.cnf", file_number);
        }
    } 
    else if (mode_choice == 2) {
        printf("Enter the full path of the CNF file: ");
        fgets(filename_out, 256, stdin);
        // 移除换行符
        size_t len = strlen(filename_out);
        if (len > 0 && filename_out[len - 1] == '\n') {
            filename_out[len - 1] = '\0';
        }
    } 
    else {
        printf("Invalid choice. Please enter 1 or 2.\n");
        return 0;
    }

    // 尝试加载文件
    if (!load_cnf_from_file(cnf, filename_out)) {
        return 0;
    }

    // 生成输出文件名
    const char* last_slash = strrchr(filename_out, '/');
    const char* last_backslash = strrchr(filename_out, '\\');
    const char* filename = filename_out;
    if (last_slash && last_backslash) {
        filename = (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
    } else if (last_slash) {
        filename = last_slash + 1;
    } else if (last_backslash) {
        filename = last_backslash + 1;
    }

    // 移除掉扩展名
    char base_name[256];
    strcpy(base_name, filename);
    char* dot = strrchr(base_name, '.');
    if (dot) *dot = '\0'; // 直接断掉
    snprintf(output_filename, 256, "res/%s.res", base_name);
    // printf("Output filename will be: %s\n", output_filename);

    return 1;
}


// 打印求解结果
void print_result(SatResult result, Assignment* assignment, double elapsed_time_ms) {
    switch (result) {
        case SAT:
            printf("Result: SATISFIABLE\n");
            // 只打印前20个变量的赋值
            printf("Assignment: ");
            for (int i = 1; i <= assignment->num_variables && i <= 20; i++) {
                if (assignment->values[i] == TRUE) {
                    printf("%d ", i);
                } else if (assignment->values[i] == FALSE) {
                    printf("%d ", -i);
                }
            }
            printf("\n");
            printf("Solving Time: %.0f ms\n", elapsed_time_ms);
            break;
            
        case UNSAT:
            printf("Result: UNSATISFIABLE\n");
            printf("Solving Time: %.0f ms\n", elapsed_time_ms);
            break;
            
        case UNKNOWN:
            printf("Result: UNKNOWN\n");
            break;
    }
}


// 把文件写到/res/下的同名.res里
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
        for (int i = 1; i <= assignment->num_variables; i++) {
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

// 用verify.exe验证结果
int verify_result(const char* cnf_file, const char* res_file) {
    char command[512];
    int verifier_choice;
    printf("\n\n\n");
    printf("Please select the verifier program (1/2): \n");
    printf("1. verify.exe\n");
    printf("2. verify5000.exe\n");
    printf("What you choice (1/2): ");
    scanf("%d", &verifier_choice);

    // 清除输入缓冲区
    while (getchar() != '\n');
    const char* verifier;
    // 保证我在ubuntu上正常运行
    if (verifier_choice == 1) {

        // verifier = "verify\\verify.exe";
#ifdef _WIN32
        verifier = "verify\\verify.exe";
#else
        verifier = "verify/verify.exe";
#endif
    } else if (verifier_choice == 2) {

#ifdef _WIN32
        verifier = "verify\\verify5000.exe";
#else
        verifier = "verify/verify5000.exe";
#endif

    } else {
        printf("Invalid choice. Skipping verification.\n");
        return 1; // 无效选择，跳过验证
    }
    
    // 构建命令行
#ifdef _WIN32
    snprintf(command, sizeof(command), "%s %s %s", verifier, cnf_file, res_file);
#else 
    // 可以让exe在ubuntu上运行的小插件
    snprintf(command, sizeof(command), "wine ./%s %s %s", verifier, cnf_file, res_file);  
#endif
    // printf("Executing command: %s\n", command);
    // 执行命令
    int ret = system(command);
    if (ret != 0) {
        printf("Verification process failed or returned non-zero exit code.\n");
        return 0;
    }
    
    return 1; // 成功
}

