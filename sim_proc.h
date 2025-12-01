#ifndef SIM_PROC_H
#define SIM_PROC_H
#include <vector>

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement

struct rmt_entry {
    bool valid = false;
    int rob_tag = -1;
};

struct rob_entry {
    bool valid = false;
    bool ready = false;
    int dst = -1;
    int global_idx = -1;
};

class ROB {
    public:
    std::vector<rob_entry> buffer;
    int head;
    int tail;
    unsigned long int count;
    unsigned long int rob_size;

    ROB(unsigned long int r_size) {
        rob_size = r_size;
        buffer.resize(rob_size);
    }

    bool available(unsigned long w) {
        return (rob_size - count) >= w;
    }

    bool isFull() {
        return count == rob_size;
    }

    bool isEmpty() {
        return count == 0;
    }

    int allocate (int global_seq, int d) {
        int rob_tag = 0;
        buffer[tail].valid = true;
        buffer[tail].ready = false;
        buffer[tail].dst = d;
        buffer[tail].global_idx = global_seq;
        count++;
        rob_tag = tail;
        tail = (tail + 1) % rob_size;
        return rob_tag;
    }


};


typedef struct timing {
    int start = -1;
    int duration = 0;
}timing;

typedef struct instruction {
    uint64_t pc;
    uint32_t seq_num;
    int op_type;
    int dest = -1;
    int src1 = -1;
    int src2 = -1;
    bool src1_ready = false;
    bool src2_ready = false;
    // rmt tags for srcs
    int src1_tag = -1;
    int src2_tag = -1;
    // index in rob that instruction is in
    // if add r2, r4, #2 and rob tag is 5, rmt gets 5 for the rob tag and valid 1 into r2
    int rob_tag = -1;

    instruction(uint64_t p, int op, int d, int s1, int s2) {
        pc = p;
        op_type = op;
        dest = d;
        src1 = s1;
        src2 = s2;
    }

    // instruction () {
    //     pc = 0;
    //     op_type = 0;
    //     dest = 0;
    //     src1 = 0;
    //     src2 = 0;
    // }
    timing FE;
    timing DE;
    timing RN;
    timing RR;
    timing DI;
    timing IS;
    timing EX;
    timing WB;
    timing RT;
}instruction;

class Pipeline_stage {
    public:

    unsigned long int width;
    std::vector<int> pipeline_instr;
     int count = 0;

    Pipeline_stage(unsigned long int w) {
        //instr.resize(w);
        width = w;
    }
    void increment_duration (std::vector<instruction> &list, timing timer);

    void fill_next_stage (Pipeline_stage* stage);

    bool full() {
        return count == width;
    }

    void clear() {
        pipeline_instr.clear();
        count = 0;
    }
    bool isEmpty() {
        if (count == 0) return true;
        else return false;
    }
};

class Simulator {
    public:
    proc_params params;
    FILE* FP;
    Pipeline_stage* FE;
    Pipeline_stage* DE;
    Pipeline_stage* RN;
    Pipeline_stage* RR;
    Pipeline_stage* DI;
    rmt_entry rmt[67];
    ROB* rob_buffer;

    Simulator(proc_params p, FILE* f) {
        params = p;
        FP = f;
        FE = new Pipeline_stage(params.width);
        DE = new Pipeline_stage(params.width);
        RN = new Pipeline_stage(params.width);
        RR = new Pipeline_stage(params.width);
        DI = new Pipeline_stage(params.width);
        rob_buffer = new ROB(params.rob_size);

        for (int i = 0; i < 67; i++) {
            rmt[i].rob_tag = -1;
            rmt[i].valid = false;
        }
    }
    void fetch();
    void decode();
    void rename();
    void RegRead();
    void dispatch();
    void issue();
    void execute();
    void write_back();
    void retire();
};
#endif
