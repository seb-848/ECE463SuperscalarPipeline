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
//rmt_entry rmt[67];

// void Pipeline_stage::increment_duration(std::vector <instruction> &list, timing &instruction::* member) {
//     for (int i = 0; i < this->pipeline_instr.size(); i++) {
//         printf("%d\n", i);
//         if (instr_list[this->pipeline_instr[i]]..start == -1) {
//             instr_list[this->pipeline_instr[i]].timer.start = global_counter;
//         }
//         else {
//             instr_list[this->pipeline_instr[i]].DE.duration++;
//         }
//     }
// }

void Pipeline_stage::fill_next_stage (Pipeline_stage* stage) {
        for (int i = 0; i < this->pipeline_instr.size(); i++) {
            stage->pipeline_instr.push_back(this->pipeline_instr[i]); stage->count++;
            //stage->pipeline_instr
        }
        this->count = 0;
        this->clear();
        return;
    }

void Simulator::fetch() {
    printf("in fetch\n");
    
    if (FE->isEmpty()) {
        uint64_t pc;
        int op_type, dest, src1, src2;
        for (int i = 0; i < params.width; i++) {
            //printf("%d \n", i);
            if (fscanf(FP, "%llx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) {
                instruction instr(pc, op_type, dest, src1, src2);
            
                instr.seq_num = fetch_seq_counter++;
                instr_list.push_back(instr);
                instr_list[fetch_seq_counter - 1].FE.start = global_counter;
                instr_list[fetch_seq_counter - 1].FE.duration++;
                printf("start for %d : %d\n",i, instr.FE.start);
                FE->pipeline_instr.push_back(instr.seq_num); FE->count++;

                // DE->pipeline_instr.push_back(instr.seq_num); DE->count++;
                // FE->clear();
                printf("seq num: %d\n", instr_list[fetch_seq_counter - 1].seq_num);
            }
        }
    }

    if (!DE->isEmpty()) {
        //FE->increment_duration(instr_list);
        for (int i = 0; i < FE->pipeline_instr.size(); i++) {
            instr_list[FE->pipeline_instr[i]].FE.duration++;
        }
        return;
    }

    FE->fill_next_stage(DE);
}

void Simulator::decode() {
    printf("in decode\n");
    if (DE->isEmpty()) return;
    
    // timing
    if (instr_list[DE->pipeline_instr[0]].DE.start == -1) {
        for (int i = 0; i < DE->pipeline_instr.size(); i++) {
            instr_list[DE->pipeline_instr[i]].DE.start = global_counter;
        }
    }
    for (int i = 0; i < DE->pipeline_instr.size(); i++) {
        instr_list[DE->pipeline_instr[i]].DE.duration++;
    }
    if (!RN->isEmpty()) return;
    DE->fill_next_stage(RN);
}

void Simulator::rename() {
    printf("in rename\n");
    if (RN->isEmpty()) return;

    // timing
    if (instr_list[RN->pipeline_instr[0]].RN.start == -1) {
        for (int i = 0; i < RN->pipeline_instr.size(); i++) {
            instr_list[RN->pipeline_instr[i]].RN.start = global_counter;
        }
    }
    for (int i = 0; i < RN->pipeline_instr.size(); i++) {
        instr_list[RN->pipeline_instr[i]].RN.duration++;
    }
//printf("still in rename\n");
    // check for rr busy or not and if space in rob
    if (!RR->isEmpty() || !rob_buffer->available(params.width)) return;
    for (int i = 0; i < RN->pipeline_instr.size(); i++) {
        instruction &current_inst = instr_list[RN->pipeline_instr[i]];

        // put into rob and get rob tag
        current_inst.rob_tag = rob_buffer->allocate(RN->pipeline_instr[i], current_inst.dest);

        // dont think we need the global sequential index
//printf("in rename loop\n");
        // getting value from arf
        if (current_inst.src1 == -1 || !rmt[current_inst.src1].valid) {
            current_inst.src1_tag = -1;
            current_inst.src1_ready = true;
        }
        else {
            current_inst.src1_tag = rmt[current_inst.src1].rob_tag;
            current_inst.src1_ready = rob_buffer->buffer[current_inst.src1_tag].ready;
        }

        if (current_inst.src2 == -1 || !rmt[current_inst.src2].valid) {
            current_inst.src2_tag = -1;
            current_inst.src2_ready = true;
        }
        else {
            current_inst.src2_tag = rmt[current_inst.src2].rob_tag;
            current_inst.src2_ready = rob_buffer->buffer[current_inst.src2_tag].ready;
        }
        
        rmt[current_inst.dest].valid = true;
        rmt[current_inst.dest].rob_tag = current_inst.rob_tag;

//printf("dest fixed\n");
        RN->fill_next_stage(RR);
        //printf("next stage filled\n");
    }
}

void Simulator::RegRead() {
    if (RR->isEmpty()) return;
    // timing
    if (instr_list[RR->pipeline_instr[0]].RR.start == -1) {
        for (int i = 0; i < RR->pipeline_instr.size(); i++) {
            instr_list[RR->pipeline_instr[i]].RR.start = global_counter;
        }
    }
    for (int i = 0; i < RR->pipeline_instr.size(); i++) {
        instr_list[RR->pipeline_instr[i]].RR.duration++;
    }

    if (!DI->isEmpty()) return;
    RR->fill_next_stage(DI);

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
        // global_counter++;
        //printf("FE empty: %d\n", sim.FE->isEmpty());
        //sim.dispatch();
        sim.RegRead();
        sim.rename();
        sim.decode();
        sim.fetch();
        
        // if (!sim.DE->isEmpty()) printf("not empty\n");
        // else printf("empty\n");
        // printf("empty: %d\n", sim.DE->isEmpty());
        global_counter++;
        //printf("%d\n", instr_list[0].FE.start);
        //printf("global counter: %llx\n", global_counter);
        test = false;
    }
    while (/*Advance_Cycle()*/global_counter < 5);

    for (int i = 0; i < instr_list.size(); i++) {
        printf("%d fu{%d} src{%d,%d} dst{%d} ", i, instr_list[i].op_type, instr_list[i].src1, instr_list[i].src2, instr_list[i].dest);

        printf("FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",
            instr_list[i].FE.start, instr_list[i].FE.duration,
            instr_list[i].DE.start, instr_list[i].DE.duration,
            instr_list[i].RN.start, instr_list[i].RN.duration,
            instr_list[i].RR.start, instr_list[i].RR.duration,
            instr_list[i].DI.start, instr_list[i].DI.duration,
            instr_list[i].IS.start, instr_list[i].IS.duration,
            instr_list[i].EX.start, instr_list[i].EX.duration,
            instr_list[i].WB.start, instr_list[i].WB.duration,
            instr_list[i].RT.start, instr_list[i].RT.duration);
    }

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
