#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "gimple.h"
#include "pass_manager.h"
#include "context.h"
#include "diagnostic-core.h"
#include "tree-pass.h"
#include "ssa.h"
#include "tree-pretty-print.h"
#include "internal-fn.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "internal-fn.h"
#include "tree-core.h"
#include "basic-block.h"

// Added headers:
#include "gimple-ssa.h"
#include "cgraph.h"
#include "attribs.h"
#include "pretty-print.h"
#include "tree-inline.h"
#include "intl.h"
#include "dumpfile.h"
#include "builtins.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>

namespace {

// Structure to store GIMPLE statement information (code and operand types)
struct GimpleStmtInfo {
    int gimple_code;                   // GIMPLE code 
    std::vector<tree> operand_types;   // Operand types
};

// Store function names and their GIMPLE codes and operand types
std::map<std::string, std::vector<GimpleStmtInfo>> function_gimple_map;

const pass_data pass_data_zwang331 = {
    GIMPLE_PASS,
    "zwang331",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_cfg,
    0,
    0,
    0,
    0,
};

class pass_zwang331 : public gimple_opt_pass {
public:
    pass_zwang331(gcc::context *ctxt)
        : gimple_opt_pass(pass_data_zwang331, ctxt) {}

    bool gate(function *) final override {
        return true;
    }

    unsigned int execute(function *fun) final override;
    static void find_and_print_cloned_functions();
};

unsigned int
pass_zwang331::execute(function *fun) {
    int bb_count = 0;
    int gimple_stmt_count = 0;
    basic_block bb;

    // Get function name
    std::string func_name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));

    // Initialize entry for this function
    function_gimple_map[func_name] = std::vector<GimpleStmtInfo>();

    if (dump_file) {
        fprintf(dump_file, "--------------------------------------------------------------------\n");
        fprintf(dump_file, "%s\n", func_name.c_str());
        fprintf(dump_file, "--------------------------------------------------------------------\n");

        FOR_EACH_BB_FN(bb, fun) {
            bb_count++;
            int bb_gimple_count = 0;

            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                bb_gimple_count++;

                // Store GIMPLE code and operand types in the map
                GimpleStmtInfo stmt_info;
                stmt_info.gimple_code = gimple_code(stmt);

                // Store operand types
                for (unsigned int i = 0; i < gimple_num_ops(stmt); i++) {
                    tree op = gimple_op(stmt, i);
                    if (op) {
                        stmt_info.operand_types.push_back(TREE_TYPE(op));
                    }
                }

                function_gimple_map[func_name].push_back(stmt_info);

                // Output GIMPLE code and operands
                fprintf(dump_file, "\n  Gimple code:       %d\n", stmt_info.gimple_code);
                fprintf(dump_file, "  Gimple code name:  %s\n", gimple_code_name[stmt_info.gimple_code]);

                for (unsigned int i = 0; i < gimple_num_ops(stmt); i++) {
                    tree op = gimple_op(stmt, i);
                    if (op) {
                        fprintf(dump_file, "  Operand:");
                        print_generic_expr(dump_file, op, TDF_NONE);
                        fprintf(dump_file, "\n");
                    }
                }

                fprintf(dump_file, "--------------------------------------------------------------------\n");
            }

            gimple_stmt_count += bb_gimple_count;
        }
    }

    if (dump_file) {
        fprintf(dump_file, "Total Basic Blocks: %d\n", bb_count);
        fprintf(dump_file, "Total GIMPLE Statements: %d\n", gimple_stmt_count);
    }

    // After all functions have been processed, find and print related functions
    find_and_print_cloned_functions();

    return 0;
}

// Function to find related functions and print their details
void pass_zwang331::find_and_print_cloned_functions() {
    if (dump_file) {
        fprintf(dump_file, "\n===== Finding Cloned Functions =====\n");
    }

    for (const auto& base_entry : function_gimple_map) {
        const std::string& base_name = base_entry.first;
        bool found_related_function = false;

        // Search for related cloned functions
        for (const auto& entry : function_gimple_map) {
            const std::string& func_name = entry.first;

            // Check if the function name starts with base_name and contains a dot (.) right after it
            // and make sure it doesn't contain ".resolver"
            if (func_name.find(base_name + ".") == 0 && func_name.find(".resolver") == std::string::npos) {
                if (!found_related_function) {
                    // Print base function's GIMPLE codes only once when a related function is found
                    if (dump_file) {
                        fprintf(dump_file, "\nBase function: %s\n", base_name.c_str());
                        fprintf(dump_file, "GIMPLE Codes: ");
                        for (const auto& stmt_info : base_entry.second) {
                            fprintf(dump_file, "%d ", stmt_info.gimple_code);
                        }
                        fprintf(dump_file, "\n---------------------------------------\n");
                    }
                    found_related_function = true;
                }

                // Compare GIMPLE codes and operand types between base function and cloned function
                bool is_same = true;
                const std::vector<GimpleStmtInfo>& base_codes = base_entry.second;
                const std::vector<GimpleStmtInfo>& cloned_codes = entry.second;

                // Compare GIMPLE codes by checking the content of each statement
                if (base_codes.size() == cloned_codes.size()) {
                    for (size_t i = 0; i < base_codes.size(); ++i) {
                        // Compare GIMPLE code first
                        if (base_codes[i].gimple_code != cloned_codes[i].gimple_code) {
                            is_same = false;
                            break;
                        }

                        // If GIMPLE code is the same, compare operand types
                        if (base_codes[i].operand_types.size() == cloned_codes[i].operand_types.size()) {
                            for (size_t j = 0; j < base_codes[i].operand_types.size(); ++j) {
                                // Compare operand types
                                if (base_codes[i].operand_types[j] != cloned_codes[i].operand_types[j]) {
                                    is_same = false;
                                    break;
                                }
                            }
                        } else {
                            is_same = false;
                        }

                        // If at any point there's a mismatch, break out of the loop
                        if (!is_same) {
                            break;
                        }
                    }
                } else {
                    is_same = false;
                }

                // Print the related cloned function's name and GIMPLE codes with comparison
                if (dump_file) {
                    fprintf(dump_file, "\nRelated cloned function: %s\n", func_name.c_str());
                    fprintf(dump_file, "GIMPLE Codes: ");
                    for (const auto& stmt_info : cloned_codes) {
                        fprintf(dump_file, "%d ", stmt_info.gimple_code);
                    }
                    fprintf(dump_file, "\n");

                    if (is_same) {
                        fprintf(dump_file, "Result: PRUNE (Codes and operand types are identical)\n");
                    } else {
                        fprintf(dump_file, "Result: NOPRUNE (Codes or operand types differ)\n");
                    }
                    fprintf(dump_file, "---------------------------------------\n");
                }
            }
        }
    }
}

} // anonymous namespace

gimple_opt_pass *
make_pass_zwang331 (gcc::context *ctxt)
{
    return new pass_zwang331 (ctxt);
}
