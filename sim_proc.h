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

    IQ(unsigned long int iq_s) {
        iq_size = iq_s;
        issue_queue.resize(iq_size);
        count = 0;
    }

    bool available(unsigned long int w) {
        return (iq_size - count) >= w;
    }

    std::vector <int> available_indices(unsigned long int w) {
        std::vector<int> indices;
        int spaces = 0;
        for (int i = 0; i < iq_size && spaces < w; i++) {
            if (!issue_queue[i].valid) {
                indices.push_back(i);// = i;
                spaces++;
            }
        }
        return indices;
    }

    bool isEmpty() {
        if (count == 0) return true;
        else return false;
    }

    int valid_entries() {
        int count = 0;
        for (int i = 0; i < iq_size; i++) {
            if (issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
                count++;
            }
        }
        return count;
    }

   std:: vector<int> oldest_up_to_width_indices(unsigned long int w) {
        std::vector <int> sorted_iq;
        //std::vector <basic_comp_entry> track;
        for (int i = 0; i < count; i++) {
            if (issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
                sorted_iq.push_back(issue_queue[i].global_idx);
            }
        }

        std::sort(sorted_iq.begin(), sorted_iq.end());

        while (sorted_iq.size() > w) {
            sorted_iq.pop_back();
        }

        for (int i = 0; i < sorted_iq.size(); i++) {
            for (int k = 0; k < issue_queue.size(); k++) {
                if (sorted_iq[i] == issue_queue[k].global_idx) {
                    sorted_iq[i] = k;
                    break;
                }
            }
        }

        return sorted_iq;
        // if (!count == 0 || !((int)issue_queue.size() == 0)) return 0;
        // int min;
        // int min_found = 0;
        // int valid_found = valid_entries();
        // if (valid_found > (int)w) valid_found = (int)w;
        // int* old_idx = new int[valid_found + 1];
        // bool used = false;

        // // populate old idx
        // if (valid_found == 0) {
        //     old_idx[0] = 0;
        //     return old_idx;
        // }
        // else {
        //     old_idx[0] = valid_found;
        //     for (int i = 1; i < valid_found + 1; i++) {
        //         old_idx[i] = -1;
        //     }
        // }
        

        // // initialize min
        // for (int i = 0; i < issue_queue.size(); i++) {
        //     if (issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
        //         min = issue_queue[i].global_idx;
        //     }
        // }

        
        // // get the absoulute min
        // for(int i = 0; i < issue_queue.size(); i++) {
        //     if (issue_queue[i].global_idx < min && issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
        //         min = issue_queue[i].global_idx;
        //         // for (int k = min_found; k < valid_found + 1; k++) {
        //         //     if (issue_queue[i].global_idx == old_idx[k] || (issue_queue[i].global_idx < old_idx[k] && old_idx[k] != )) {
        //         //         used = true;
        //         //         break;
        //         //     }
        //         //     else if (k == valid_found + 1) {

        //         //     }
        //         // }
        //         // if ()
        //         old_idx[1] = min;
        //     }
        // }

        // min_found++;

        // if (valid_found == 1) return old_idx;

        // for(int i = 0; i < issue_queue.size() && min_found < w; i++) {

        // }



        //printf("IN FUNC\n");
    //     int min = issue_queue[0].global_idx;
    //     int count = 1;
    //     int entries_taken = 0;
    //     int take_num = w;
        
    //     bool used = false;

    //     for (int i = 0; i < iq_size; i++) {
    //         if (issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
    //             entries_taken++;
    //         }
    //     }

    //     if (entries_taken < w) take_num = entries_taken;
    //     int* indices = new int[take_num + 1]();
    //     for (int i = 1; i < take_num + 1; i++) {
    //         indices[i] = -1;
    //     }
    //     indices[0] = take_num + 1;
    //     // printf("GETTING OLDEST INST\n");
    //     // printf("first index for finding oldest in func %d\n", indices[0]);
    //     while(count <= take_num) {
    //         for (int i = 0; i < iq_size; i++) {
    //             if (issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
    //                 //printf("valid entry found, seq: %d\n", issue_queue[i].global_idx);
    //                 if (min >= issue_queue[i].global_idx) {
    //                     for (int k = 1; k < indices[0]; k++) {
    //                         if (indices[k] == issue_queue[i].global_idx && indices[k] != -1) {
    //                             used = true;
    //                             //printf("NOT UNIQUE\n");
    //                             break;
    //                         }
    //                     }
    //                     if (!used)  {
    //                         min = issue_queue[i].global_idx;
    //                         used = false;
    //                         indices[count++] = min;
    //                         min++;
                            
    //                         //printf("REPLACING MIN\n");
    //                         break;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     return indices;
    // }
   }
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
    int retired = -1;
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

//template <typename T>
class Pipeline_stage {
    public:

    unsigned long int width = 0;

    std::vector<int> pipeline_instr;
     int count = 0;

    Pipeline_stage(unsigned long int w) {
        //pipeline_instr.resize(w);
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
        return count == width;
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
        iq_str = new IQ(params.iq_size);
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
    void issue();
    void execute();
    void write_back();
    void retire();
};
#endif
