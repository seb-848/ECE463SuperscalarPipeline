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
rmt_entry rmt[67];

void Simulator::fetch() {
    if (!DE->isEmpty()) return;
    uint64_t pc;
    int op_type, dest, src1, src2;
    for (int i = 0; i < params.width; i++) {
        if (fscanf(FP, "%llx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) {
            instruction instr(pc, op_type, dest, src1, src2);
            instr.seq_num = fetch_seq_counter++;
            instr_list.push_back(instr);
            instr.FE.start = global_counter;
            printf("%d\n", instr.seq_num);
            DE->instr[DE->count++] = instr.seq_num;
            //printf("%llx %d %d %d %d\n", instr.pc, instr.op_type, instr.dest, instr.src1,instr.src2); //Print to check if inputs have been read correctly
        }
        else {
            return;
        }
    }
}

void Simulator::decode() {
    if (DE->isEmpty()) return;
    for (int i = 0; i < DE->instr.size(); i++) {
        if (instr_list[DE->instr[i]].DE.start == -1) {
            instr_list[DE->instr[i]].DE.start = global_counter;
        }
        else {
            instr_list[DE->instr[i]].DE.duration++;
        }
    }
    if (!RN->isEmpty()) return;
    for (int i = 0; i < DE->instr.size(); i++) {
        RN->instr[RN->count++] = DE->instr[i];
    }
    DE->clear();
}

void Simulator::rename() {
    if (RN->isEmpty()) return;

    for (int i = 0; i < RN->instr.size(); i++) {
        if (instr_list[RN->instr[i]].RN.start == -1) {
            instr_list[RN->instr[i]].RN.start = global_counter;
        }
        else {
            instr_list[RN->instr[i]].RN.duration++;
        }
    }

    if (!RR->isEmpty()) return;

    //if ()
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
    // Pipeline_stage* DE = new Pipeline_stage(params.width);
    // Pipeline_stage* RN = new Pipeline_stage(params.width);
    // Pipeline_stage* RR = new Pipeline_stage(params.width);
    // Pipeline_stage* DI = new Pipeline_stage(params.width);
    //pipeline_stage WB;
    Simulator sim(params, FP);
    //Pipeline_stage* DE = new Pipeline_stage(params.width);
    bool test = true;
    do {
        global_counter++;
        
        sim.rename();
        sim.decode();
        sim.fetch();
        
        // if (!sim.DE->isEmpty()) printf("not empty\n");
        // else printf("empty\n");
        // printf("empty: %d\n", sim.DE->isEmpty());
        test = false;
    }
    while (/*Advance_Cycle()*/test);

    // while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
    // {
    //     printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
    // }
    // delete(DE);
    // delete(RN);
    // delete(RR);
    // delete(DI);
    
    return 0;
}
