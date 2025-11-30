#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
//#include <cinttypes>
#include "sim_proc.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
uint64_t global_counter = 0; // global cycle counter
uint64_t fetch_seq_counter = 0;
std::vector<instruction> instr_list; //global  instruction list

void fetch(proc_params params, FILE* FP) {
    uint64_t pc;
    int op_type, dest, src1, src2;
    for (int i = 0; i < params.width; i++) {
        if (fscanf(FP, "%llx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) {
            instruction instr(pc, op_type, dest, src1, src2);
            instr.seq_num = fetch_seq_counter++;
            instr_list.push_back(instr);
            printf("%d\n", instr.seq_num);
            //printf("%llx %d %d %d %d\n", instr.pc, instr.op_type, instr.dest, instr.src1,instr.src2); //Print to check if inputs have been read correctly
        }
    }
}

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    pipeline_stage* DE = new pipeline_stage(params.width);
    pipeline_stage* RN = new pipeline_stage(params.width);
    pipeline_stage* RR = new pipeline_stage(params.width);
    pipeline_stage* DI = new pipeline_stage(params.width);
    //pipeline_stage WB;
    bool test = true;
    do {
        global_counter++;
        // for (int i = 0; i < params.width; i++) {
        //     if (fscanf(FP, "%llx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) {
        //     instruction instr(pc, op_type, dest, src1, src2);
        //     instr.seq_num = fetch_seq_counter++;
        //     instr_list.push_back(instr);
        //     printf("%d\n", instr.seq_num);
        //     //printf("%llx %d %d %d %d\n", instr.pc, instr.op_type, instr.dest, instr.src1,instr.src2); //Print to check if inputs have been read correctly
        //     }
        //     else {
        //     test = false;
        // }
        // }
        fetch(params,  FP);
        test = false;
    }
    while (/*Advance_Cycle()*/test);

    // while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
    // {
    //     printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
    // }
    delete(DE);
    delete(RN);
    delete(RR);
    delete(DI);
    
    return 0;
}
