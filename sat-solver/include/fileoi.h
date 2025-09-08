#ifndef FILEOI_H
#define FILEOI_H

#include "dynamic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =========== 文件操作函数 ===========
int load_cnf_from_file(CNF* cnf, const char* filename);
int load_cnf_interactive(CNF* cnf, char* filename_out, char *output_filename);
void print_result(SatResult result, Assignment* assignment, double elapsed_time_ms);
void save_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms);

int verify_result(const char* cnf_file, const char* res_file);

#endif // FILEOI_H
