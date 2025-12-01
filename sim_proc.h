#ifndef SIM_PROC_H
#define SIM_PROC_H
#include <vector>

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement
class Pipeline_stage {
    public:

    unsigned long int width;
    std::vector<int> instr;
    unsigned long int count = 0;

    Pipeline_stage(unsigned long int w) {
        //instr.resize(w);
        width = w;
    }

    bool full() {
        return count == width;
    }

    void clear() {
        instr.clear();
    }
    bool isEmpty() {
        if (count == 0) return true;
        else return false;
    }
};
struct rmt_entry {
    bool valid = false;
    int rob_tag = -1;
};

struct rob_entry {
    bool valid = false;
    bool ready = false;
    int dst = -1;
    int instruction = -1;
};

class ROB {
    public:

};


typedef struct timing {
    int start = -1;
    int duration = 0;
}timing;

typedef struct instruction {
    uint64_t pc;
    uint32_t seq_num;
    int op_type;
    int dest;
    int src1;
    int src2;
    bool src1_ready = false;
    bool src2_ready = false;
    int src1_tag = -1;
    int src2_tag = -1;
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

class Simulator {
    public:
    proc_params params;
    FILE* FP;
    Pipeline_stage* DE;
    Pipeline_stage* RN;
    Pipeline_stage* RR;
    Pipeline_stage* DI;

    Simulator(proc_params p, FILE* f) {
        params = p;
        FP = f;
        DE = new Pipeline_stage(params.width);
        RN = new Pipeline_stage(params.width);
        RR = new Pipeline_stage(params.width);
        DI = new Pipeline_stage(params.width);
    }
    void fetch();
    void decode();
    void rename();
    void register_read();
    void dispatch();
    void issue();
    void execute();
    void write_back();
    void retire();
};
#endif
