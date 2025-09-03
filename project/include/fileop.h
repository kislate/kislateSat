#ifndef FILEOF_H
#define FILEOF_H
#include "sat_solver.h"

// Load CNF from file
int load_cnf_from_file(CNF* cnf, const char* filename); 

// Interactive load CNF with user input
int load_cnf_interactive(CNF* cnf, char* filename_out);

// Save result to file
void save_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms);

// Verify result function
int verify_result(const char* cnf_file, const char* res_file);

// Save and print result in one step
void save_and_print_result(const char* filename, SatResult result, const Assignment* assignment, double elapsed_time_ms);

#endif // FILEOF_H