#ifndef SIM_PROC_H
#define SIM_PROC_H
#include <vector>
#include <algorithm>

int const EX_LIST_LIMIT_FACTOR = 5;

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement
struct iq_entry {
    bool valid = false;
    int rob_tag = -1;
    bool src1_ready = false;
    bool src2_ready = false;
    int src1_tag = -1;
    int src2_tag = -1;
    int global_idx = -1;
};

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
struct basic_comp_entry {
    int global_idx = -1;
    int iq_idx = -1;
};
class IQ {
    public:
    std::vector<iq_entry> issue_queue;
    unsigned long int count = 0;;
    unsigned long int iq_size = 0;;

    IQ(unsigned long int iq_s, iq_entry def_iq_val) {
        iq_size = iq_s;
        issue_queue.assign(iq_size, def_iq_val);
        count = 0;
    }

    bool available(int width = 1) {
        int count = 0;
        for (int i = 0; i < iq_size; i++) {
            if (!issue_queue[i].valid) {
                count++;
                //return true;
            }
        }
        if (count >= width) return true;
        else return false;
    }

    std::vector <int> available_indices(unsigned long int w) {
        std::vector<int> indices;
        int spaces = 0;
        for (int i = 0; i < iq_size && spaces < w; i++) {
            if (!issue_queue[i].valid) {
                indices.push_back(i);
                spaces++;
            }
        }
        return indices;
    }

    bool isEmpty() {
        //for (int i = 0; i < )
        if (count == 0) return true;
        else return false;
    }

    int valid_entries();

    int index_find(int inst_global_index) {
        for (int i = 0; i < issue_queue.size(); i++) {
            if (issue_queue[i].global_idx == inst_global_index) {
                return i;
            }
        }
        return -1;
    }

    std:: vector<int> oldest_up_to_width_indices(unsigned long int w, int space);
};

class ROB {
    public:
    std::vector<rob_entry> buffer;
    int head = 0;
    int tail = 0;
    unsigned long int count = 0;
    unsigned long int rob_size = 0;

    ROB(unsigned long int r_size) {
        rob_size = r_size;
        buffer.resize(rob_size);
        count = 0;
    }

    bool available(unsigned long w) {
        return (rob_size - count) >= w;
    }

    bool isFull() {
        return count == rob_size;
    }

    bool isEmpty() {
        if (count == 0) return true;
        else return false;
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
    bool retired = false;
    int rob_tag = -1;

    instruction(uint64_t p, int op, int d, int s1, int s2) {
        pc = p;
        op_type = op;
        dest = d;
        src1 = s1;
        src2 = s2;
    }
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

    unsigned long int width = 0;

    std::vector<int> pipeline_instr;
     int count = 0;

    Pipeline_stage(unsigned long int w) {
        width = w;
    }
    //void increment_duration (std::vector<instruction> &list, timing timer);

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

    void move_next_stage(Pipeline_stage* stage);
};

struct execute_entry {
    int time_left = -1;
    int global_idx = -1;
};
class EX_Stage {
    public:

    unsigned long int width = 0;

    std::vector<execute_entry> execute_list;
    int count = 0;

    EX_Stage(unsigned long int w) {
        //instr.resize(w);
        width = w;
    }

    bool space_available(int bundle_size) {
        if ((execute_list.size() + bundle_size) < (width * EX_LIST_LIMIT_FACTOR)) return true;
        else return false;
    }
    bool full() {
        return count == width * EX_LIST_LIMIT_FACTOR;
    }

    void clear() {
        execute_list.clear();
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
    Pipeline_stage* IS;
    EX_Stage* EX;
    Pipeline_stage* WB;
    Pipeline_stage* RT;
    std::vector<rmt_entry> rmt;
    ROB* rob_buffer;
    IQ* iq_str;
    iq_entry def;

    Simulator(proc_params p, FILE* f) {
        params = p;
        FP = f;
        FE = new Pipeline_stage(params.width);
        DE = new Pipeline_stage(params.width);
        RN = new Pipeline_stage(params.width);
        RR = new Pipeline_stage(params.width);
        DI = new Pipeline_stage(params.width);
        IS = new Pipeline_stage(params.width);
        EX = new EX_Stage(params.width);
        WB = new Pipeline_stage(params.width);
        RT = new Pipeline_stage(params.width);
        rob_buffer = new ROB(params.rob_size);
        iq_str = new IQ(params.iq_size, def);
        rmt.resize(67);
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
    void iq_func();
    void issue();
    void execute();
    void write_back();
    void retire();
    bool advance_cycle();
};
#endif
