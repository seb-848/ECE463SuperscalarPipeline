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
bool fetch_done = false;
std::vector<int> already_retired;

void Pipeline_stage::move_next_stage(Pipeline_stage* stage) {
    if (!stage->full()) {
        int available = stage->width - stage->pipeline_instr.size();
        int moved = 0;
        for (int i = 0; i < available && i < pipeline_instr.size(); i++) {
            stage->pipeline_instr.push_back(pipeline_instr[i]);
            //stage->count++;
            moved++;
        }

        for (int i = 0; i < moved; i++) {
            pipeline_instr.erase(pipeline_instr.begin());
        }
    }
    count = pipeline_instr.size();
    stage->count = stage->pipeline_instr.size();
    return;
}
int IQ::valid_entries() {
        int count = 0;
        for (int i = 0; i < issue_queue.size(); i++) {
            if (issue_queue[i].valid) {
                if (instr_list[issue_queue[i].global_idx].src1 != -1) {
                    if (issue_queue[i].src1_ready) {
                        if (instr_list[issue_queue[i].global_idx].src2 != -1) {
                            if (issue_queue[i].src2_ready) {
                                count++;
                                continue;
                            }
                            else {
                                continue;
                            }
                        }
                    } 
                    else {
                        continue;
                    }
                }
                if (instr_list[issue_queue[i].global_idx].src2 != -1) {
                    if (issue_queue[i].src2_ready) {
                        count++;
                    }
                    else {
                        continue;
                    }
                }
                else {
                    count++;
                    continue;
                }
            }
        }
        return count;
    }

std:: vector<int> IQ::oldest_up_to_width_indices(unsigned long int w, int space) {
        std::vector <int> sorted_iq;
        //int valid = valid_entries();
        //bool ready_inst = true;
        int valid = 0;
        
        
        if (space <= 0) return std::vector <int>();
        
        for (int i = 0; i < issue_queue.size(); i++) {
            bool ready_inst = true;
            if (issue_queue[i].valid) {
                if (instr_list[issue_queue[i].global_idx].src1 != -1) {
                    if (!issue_queue[i].src1_ready) {
                        ready_inst = false;
                    }
                }

                if (instr_list[issue_queue[i].global_idx].src2 != -1) {
                    if (!issue_queue[i].src2_ready) {
                        ready_inst = false;
                    }
                }

                if (ready_inst) {
                    sorted_iq.push_back(issue_queue[i].global_idx);
                    valid++;
                }
            }
        }

        std::sort(sorted_iq.begin(), sorted_iq.end());
        //printf("valid #: %d\n",valid);
        
        //if (valid > (int)w) valid = (int)w;
        if (valid <= 0) return std::vector <int>();
        
        
        int limit = 0;
        if (space > w) limit = w;
        else limit = space;
        
        if ((int)sorted_iq.size() > limit) sorted_iq.resize(limit);

        for (int i = 0; i < sorted_iq.size(); i++) {
            for (int k = 0; k < issue_queue.size(); k++) {
                if (sorted_iq[i] == issue_queue[k].global_idx) {
                    sorted_iq[i] = k;
                    break;
                }
            }
        }
        return sorted_iq;
   }
bool Simulator::advance_cycle() {
    bool advance = true;

    if (fetch_done && rob_buffer->isEmpty() && iq_str->isEmpty() && EX->isEmpty() && WB->isEmpty() && RT->isEmpty()) {
        if (FE->isEmpty() && DE->isEmpty() && RN->isEmpty() && RR->isEmpty() && DI->isEmpty()) {
            advance = false;
        }
    }
    return advance;
}

void Pipeline_stage::fill_next_stage (Pipeline_stage* stage) {
        for (int i = 0; i < this->count; i++) {
            stage->pipeline_instr.push_back(this->pipeline_instr[i]); stage->count++;
            //stage->pipeline_instr
        }
        this->count = 0;
        this->clear();
        return;
    }

void Simulator::fetch() {
    //printf("in fetch\n");
    //timing
    if (!DE->isEmpty()) {
        for (int i = 0; i < FE->count; i++) {
            instr_list[FE->pipeline_instr[i]].FE.duration++;
        }
        return;
    }
    
    if (FE->isEmpty()) {
        uint64_t pc;
        int op_type, dest, src1, src2;
        for (int i = 0; i < params.width; i++) {
            //printf("%d \n", i);
            if (fscanf(FP, "%llx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) {
                instruction instr(pc, op_type, dest, src1, src2);
        
                if (fetch_seq_counter == 0) {
                    if (instr.src1 != -1) {
                        instr.src1_ready = true;
                    }
                    if (instr.src2 != -1) {
                        instr.src2_ready = true;
                    }
                }
                instr.seq_num = fetch_seq_counter++;
                instr_list.push_back(instr);
                instr_list[fetch_seq_counter - 1].FE.start = global_counter;
                instr_list[fetch_seq_counter - 1].FE.duration++;
                //printf("start for %d : %d\n",i, instr.FE.start);
                FE->pipeline_instr.push_back(instr.seq_num); FE->count++;
            }
            else {
                fetch_done = true;
            }
        }
    }

    FE->move_next_stage(DE);
    return;
}

void Simulator::decode() {
    //printf("in decode\n");
    if (DE->isEmpty()) return;
    
    // timing
    if (instr_list[DE->pipeline_instr[0]].DE.start == -1) {
        for (int i = 0; i < DE->count; i++) {
            instr_list[DE->pipeline_instr[i]].DE.start = global_counter;
        }
    }
    for (int i = 0; i < DE->pipeline_instr.size(); i++) {
        instr_list[DE->pipeline_instr[i]].DE.duration++;
    }
    // if (!RN->isEmpty()) return;
    // DE->fill_next_stage(RN);
    DE->move_next_stage(RN);
    return;
}

void Simulator::rename() {
    //printf("in rename\n");
    if (RN->isEmpty()) return;

    // timing
    // if (instr_list[RN->pipeline_instr[0]].RN.start == -1) {
    //     for (int i = 0; i < RN->pipeline_instr.size(); i++) {
    //         instr_list[RN->pipeline_instr[i]].RN.start = global_counter;
    //     }
    // }
    // for (int i = 0; i < RN->pipeline_instr.size(); i++) {
    //     instr_list[RN->pipeline_instr[i]].RN.duration++;
    // }

    for (int i = 0; i < RN->pipeline_instr.size(); i++) {
    if (instr_list[RN->pipeline_instr[i]].RN.start == -1) {
        instr_list[RN->pipeline_instr[i]].RN.start = global_counter;
    }
    instr_list[RN->pipeline_instr[i]].RN.duration++;
   }
//printf("still in rename\n")//;
    
    int not_in_rob = 0;
    for (int i = 0; i < RN->pipeline_instr.size(); i++) {
        if (instr_list[RN->pipeline_instr[i]].rob_tag == -1) not_in_rob++;
    }
    if (not_in_rob > 0 && !rob_buffer->available(not_in_rob)) return;

    for (int i = 0; i < RN->pipeline_instr.size(); i++) {
        instruction &current_inst = instr_list[RN->pipeline_instr[i]];
        
        // rob entry already
        if (current_inst.rob_tag != -1) continue;
        
        // put into rob and get rob tag
        current_inst.rob_tag = rob_buffer->allocate(RN->pipeline_instr[i], current_inst.dest);

//printf("in rename loop\n");
        //printf("rmt src1 check: %d\n",rmt[current_inst.src1].valid);
        if (current_inst.src1 == -1 || !rmt[current_inst.src1].valid) {
            current_inst.src1_tag = -1;
            current_inst.src1_ready = true;
            current_inst.src1_producer_seq = -1;
        }
        else {
            current_inst.src1_tag = rmt[current_inst.src1].rob_tag;
            current_inst.src1_ready = rob_buffer->buffer[current_inst.src1_tag].ready;
            current_inst.src1_producer_seq = rob_buffer->buffer[current_inst.src1_tag].global_idx;
        }

        if (current_inst.src2 == -1 || !rmt[current_inst.src2].valid) {
            current_inst.src2_tag = -1;
            current_inst.src2_ready = true;
            current_inst.src2_producer_seq = -1;
        }
        else {
            current_inst.src2_tag = rmt[current_inst.src2].rob_tag;
            current_inst.src2_ready = rob_buffer->buffer[current_inst.src2_tag].ready;
            current_inst.src2_producer_seq = rob_buffer->buffer[current_inst.src2_tag].global_idx;
        }
        
        if (current_inst.dest != -1) {
            rmt[current_inst.dest].valid = true;
            rmt[current_inst.dest].rob_tag = current_inst.rob_tag;
        }
        

//printf("dest fixed\n");
        //RN->fill_next_stage(RR);
        //printf("next stage filled\n");
    }
    //RN->fill_next_stage(RR);
    RN->move_next_stage(RR);
    return;
}

void Simulator::RegRead() {
    //printf("in regread\n");
    if (RR->isEmpty()) return;
    // timing
    // if (instr_list[RR->pipeline_instr[0]].RR.start == -1) {
    //     for (int i = 0; i < RR->pipeline_instr.size(); i++) {
    //         instr_list[RR->pipeline_instr[i]].RR.start = global_counter;
    //     }
    // }
    // for (int i = 0; i < RR->pipeline_instr.size(); i++) {
    //     instr_list[RR->pipeline_instr[i]].RR.duration++;
    // }
    for (int i = 0; i < RR->pipeline_instr.size(); i++) {
    if (instr_list[RR->pipeline_instr[i]].RR.start == -1) {
        instr_list[RR->pipeline_instr[i]].RR.start = global_counter;
    }
    instr_list[RR->pipeline_instr[i]].RR.duration++;
   }
    // if (!DI->isEmpty()) return;
    // RR->fill_next_stage(DI);
    RR->move_next_stage(DI);
    return;
}

void Simulator::dispatch() {
    //printf("in dispatch\n");
    if (DI->isEmpty()) return;
    for (int i = 0; i < DI->pipeline_instr.size(); i++) {
    if (instr_list[DI->pipeline_instr[i]].DI.start == -1) {
        instr_list[DI->pipeline_instr[i]].DI.start = global_counter;
    }
    instr_list[DI->pipeline_instr[i]].DI.duration++;
   }

    
    bool available_space = iq_str->available(DI->pipeline_instr.size());
    if (!available_space) return;

    int moved = 0;
    for (int i = 0; i < (int)DI->pipeline_instr.size() && moved < (int)params.width; i++) {
        int filling_index = -1;
        instruction &current_inst = instr_list[DI->pipeline_instr[i]];
        for (int k = 0; k < (int)iq_str->issue_queue.size(); k++) {
            if (!iq_str->issue_queue[k].valid) {
                filling_index = k;
                break;
            }
        }

        if (filling_index == -1) break;
        iq_str->issue_queue[filling_index].valid = true;
        iq_str->issue_queue[filling_index].rob_tag = current_inst.rob_tag;
        iq_str->issue_queue[filling_index].src1_ready = current_inst.src1_ready;
        iq_str->issue_queue[filling_index].src1_tag = current_inst.src1_tag;
        iq_str->issue_queue[filling_index].src2_ready = current_inst.src2_ready;
        iq_str->issue_queue[filling_index].src2_tag = current_inst.src2_tag;
        iq_str->issue_queue[filling_index].global_idx = DI->pipeline_instr[i];
        iq_str->count++;
        moved++;

        if (iq_str->issue_queue[filling_index].src1_tag != -1) {
            // check rob is still expected producer
            if (rob_buffer->buffer[iq_str->issue_queue[filling_index].src1_tag].global_idx == current_inst.src1_producer_seq) {
                if (rob_buffer->buffer[iq_str->issue_queue[filling_index].src1_tag].ready) {
                    iq_str->issue_queue[filling_index].src1_ready = true;
                }
            } else {
                // new rob src ready
                iq_str->issue_queue[filling_index].src1_ready = true;
            }
        }

        if (iq_str->issue_queue[filling_index].src2_tag != -1) {
            if (rob_buffer->buffer[iq_str->issue_queue[filling_index].src2_tag].global_idx == current_inst.src2_producer_seq) {
                if (rob_buffer->buffer[iq_str->issue_queue[filling_index].src2_tag].ready) {
                    iq_str->issue_queue[filling_index].src2_ready = true;
                }
            } else {
                iq_str->issue_queue[filling_index].src2_ready = true;
            }
        }

        
        //printf("filling iq\n");
    }
    
    //DI->fill_next_stage(IS);
    //printf("clearing  dispatch\n");
    //DI->clear();
    for (int i = 0; i < moved; i++) {
        DI->pipeline_instr.erase(DI->pipeline_instr.begin());
    }
    DI->count = DI->pipeline_instr.size();
    //delete[] index_to_fill;
    return;
}

void Simulator::iq_func() {
    //if (iq_str->isEmpty()) return;
    //if (IS->full()) return;
    //printf("in iq func\n");
    for (int i = 0; i < (int)iq_str->issue_queue.size(); i++) {
        if (iq_str->issue_queue[i].valid) {
            if (instr_list[iq_str->issue_queue[i].global_idx].IS.start == -1) {
                instr_list[iq_str->issue_queue[i].global_idx].IS.start = global_counter;
            }
            instr_list[iq_str->issue_queue[i].global_idx].IS.duration++;
        }
    }

    
    int available = params.width - IS->pipeline_instr.size();
    //printf("available space in is %d\n", available);
    std::vector <int> indicies = iq_str->oldest_up_to_width_indices(params.width, available);
    //int available = params.width - IS->pipeline_instr.size();
    //printf("indicies size: %d\n", indicies.size());
    for (int i = 0; i < indicies.size(); i++) {
        //printf("filling issue\n");
        iq_entry &current_iq_entry = iq_str->issue_queue[indicies[i]];
        //printf("current entry made %d\n", current_iq_entry.global_idx);
        //add to issue stage to put into execute list
        IS->pipeline_instr.push_back(current_iq_entry.global_idx);
        //IS->count++;
        //printf("entry transferred\n");
        //remove from iq
        current_iq_entry.rob_tag = -1;
        current_iq_entry.valid = false;
        current_iq_entry.src1_ready = false;
        current_iq_entry.src1_tag = -1;
        current_iq_entry.src2_ready = false;
        current_iq_entry.src2_tag = -1;
        current_iq_entry.global_idx = -1;
        iq_str->count--;
        ++IS->count;
        
        //printf("issue being filled\n");
    }
    //iq_str->count = iq_str->issue_queue.size();
    IS->count = IS->pipeline_instr.size();
    //issue();
    return;
}

void Simulator::issue() {
   // printf("in issue\n");
  // if (IS->isEmpty()) return;
  if (iq_str->isEmpty()) return;
//    for (int i = 0; i < IS->pipeline_instr.size(); i++) {
//     if (instr_list[IS->pipeline_instr[i]].IS.start == -1) {
//         instr_list[IS->pipeline_instr[i]].IS.start = global_counter;
//     }
//     instr_list[IS->pipeline_instr[i]].IS.duration++;
//    }
    for (int i = 0; i < (int)iq_str->issue_queue.size(); i++) {
        if (iq_str->issue_queue[i].valid) {
            if (instr_list[iq_str->issue_queue[i].global_idx].IS.start == -1) {
                instr_list[iq_str->issue_queue[i].global_idx].IS.start = global_counter;
            }
            instr_list[iq_str->issue_queue[i].global_idx].IS.duration++;
        }
    }

    
    int available = params.width - IS->pipeline_instr.size();
    //printf("available space in is %d\n", available);
    std::vector <int> indicies = iq_str->oldest_up_to_width_indices(params.width, available);
    //int available = params.width - IS->pipeline_instr.size();
    //printf("indicies size: %d\n", indicies.size());
    for (int i = 0; i < indicies.size(); i++) {
        //printf("filling issue\n");
        iq_entry &current_iq_entry = iq_str->issue_queue[indicies[i]];
        //printf("current entry made %d\n", current_iq_entry.global_idx);
        //add to issue stage to put into execute list
        IS->pipeline_instr.push_back(current_iq_entry.global_idx);
        //IS->count++;
        //printf("entry transferred\n");
        //remove from iq
        current_iq_entry.rob_tag = -1;
        current_iq_entry.valid = false;
        current_iq_entry.src1_ready = false;
        current_iq_entry.src1_tag = -1;
        current_iq_entry.src2_ready = false;
        current_iq_entry.src2_tag = -1;
        current_iq_entry.global_idx = -1;
        iq_str->count--;
        ++IS->count;
        
        //printf("issue being filled\n");
    }
    //iq_str->count = iq_str->issue_queue.size();
    IS->count = IS->pipeline_instr.size();

    if (!EX->full()) {
        int moved = 0;
        int available = (EX_LIST_LIMIT_FACTOR * params.width) - EX->execute_list.size();
        for (int i = 0; i < IS->pipeline_instr.size(); i++) {
            execute_entry ex_list_entry;
            ex_list_entry.global_idx = IS->pipeline_instr[i];
            switch(instr_list[IS->pipeline_instr[i]].op_type) {
                case 0:
                ex_list_entry.time_left = 1;
                break;
                case 1:
                ex_list_entry.time_left = 2;
                break;
                case 2:
                ex_list_entry.time_left = 5;
                break;
                default:
                break;
            }
            EX->execute_list.push_back(ex_list_entry);
            //printf("ex entry from is: %d\n", ex_list_entry.global_idx);
            //EX->count++;
            moved++;
        }
        for (int i = 0; i < moved; i++) {
            IS->pipeline_instr.erase(IS->pipeline_instr.begin());
        }
        IS->count = IS->pipeline_instr.size();
        EX->count = EX->execute_list.size();
    }
    return;
}

void Simulator::execute() {
    //printf("in execute\n");
    if (EX->isEmpty()) return;
    //printf("ex not empty\n");
    //timing
    
    // if (instr_list[EX->execute_list[0].global_idx].EX.start == -1) {
    //     for (int i = 0; i < EX->execute_list.size(); i++) {
    //         instr_list[EX->execute_list[i].global_idx].EX.start = global_counter;
    //     }
    // }
    for (int i = 0; i < EX->execute_list.size(); i++) {
        if (instr_list[EX->execute_list[i].global_idx].EX.start == -1) {
            instr_list[EX->execute_list[i].global_idx].EX.start = global_counter;
        }
    }
    for (int i = 0; i < EX->execute_list.size(); i++) {
        instr_list[EX->execute_list[i].global_idx].EX.duration++;
        if (EX->execute_list[i].time_left > 0) {
            EX->execute_list[i].time_left--;
        }
    }

    //std::vector <int> executed_inst;
    int execute_count = 0;

    //printf("ex is about to check to move to wb\n");
    // check for instructions done and add to WB
    std::vector<int> removed;
    for (int i = 0; i < EX->execute_list.size(); i++) {
        //printf("about to check for timing\n");
        if (EX->execute_list[i].time_left == 0) {
            WB->pipeline_instr.push_back(EX->execute_list[i].global_idx);
            WB->count++;
            execute_count++;
            removed.push_back(i);
            instruction &current_inst = instr_list[EX->execute_list[i].global_idx];
            
            // wake ups for IQ, DI, and RR
            for (int k = 0; k < iq_str->iq_size; k++) {
                if (iq_str->issue_queue[k].valid){
                    if (iq_str->issue_queue[k].src1_tag == current_inst.rob_tag) {
                        iq_str->issue_queue[k].src1_ready = true;
                    }
                    if (iq_str->issue_queue[k].src2_tag == current_inst.rob_tag) {
                        iq_str->issue_queue[k].src2_ready = true;
                    }
                }
            //printf("out of editing of iq\n");
            }

            for (int k = 0; k < DI->pipeline_instr.size(); k++) {
                if (instr_list[DI->pipeline_instr[k]].src1_tag == current_inst.rob_tag) {
                    instr_list[DI->pipeline_instr[k]].src1_ready = true;
                }
                if (instr_list[DI->pipeline_instr[k]].src2_tag == current_inst.rob_tag) {
                    instr_list[DI->pipeline_instr[k]].src2_ready = true;
                }
            }

            for (int k = 0; k < RR->pipeline_instr.size(); k++) {
                if (instr_list[RR->pipeline_instr[k]].src1_tag == current_inst.rob_tag) {
                    instr_list[RR->pipeline_instr[k]].src1_ready = true;
                }
                if (instr_list[RR->pipeline_instr[k]].src2_tag == current_inst.rob_tag) {
                    instr_list[RR->pipeline_instr[k]].src2_ready = true;
                }
            }
        }
    }

    //printf("transfer all done\n");
    

    // printf("global counter %d\n", global_counter);
    // for (int i = 0; i < execute_count; i++) {
    //     printf("EX contents i: %d, content: %d\n", i, EX->execute_list[i].global_idx);
    // }
    //int removed = 0;

    // remove execute instructions that were added to WB
    EX->count = EX->execute_list.size();
    for (int i = 0; i < execute_count; i++) {
        EX->execute_list.erase(EX->execute_list.begin() + removed[i] - i);
    }
    // for (int i = EX->count - 1; removed < execute_count && i >= 0; i--) {
    //     if (EX->execute_list[i].time_left == 0) {
    //         EX->execute_list.erase(EX->execute_list.begin() + i);
    //         ++removed;
    //     }
    //     //EX->execute_list.erase(EX->execute_list.begin());
    // }
    EX->count = (int)EX->execute_list.size();
    // RT->count = (int)RT->pipeline_instr.size();
    // WB->count = (int)WB->pipeline_instr.size();
    //execute_count = 0;//ithink
    //EX
    // while (execute_count != 0) {
    //     EX->execute_list.erase(EX->execute_list.begin() + executed_inst[execute_count - 1]);
    //     --execute_count;
    // }
    //execute_count = 0;
    
    return;
}

void Simulator::write_back() {
    if (WB->isEmpty()) return;
    //timing
    if (instr_list[WB->pipeline_instr[0]].WB.start == -1) {
        for (int i = 0; i < WB->count; i++) {
            instr_list[WB->pipeline_instr[i]].WB.start = global_counter;
        }
    }
    for (int i = 0; i < WB->count; i++) {
        instr_list[WB->pipeline_instr[i]].WB.duration++;
    }

    //printf("timing done in write back\n");
    //printf("WB count: %d\n", WB->count);
    // update rob to be ready for executed instruction
    for (int i = 0; i < WB->count; i++) {
        instruction &current_inst = instr_list[WB->pipeline_instr[i]];
        rob_buffer->buffer[current_inst.rob_tag].ready = true;
        
        //printf("about to enter the editing of issue queue\n");

        // for (int k = 0; k < iq_str->iq_size; k++) {
        //     //printf("in editing of iq\n");
        //     //printf("iq seq: %d    current inst seq: %d\n",iq_str->issue_queue[k].global_idx, current_inst.seq_num);
        //     // if (iq_str->issue_queue[k].global_idx == current_inst.seq_num) {
        //     //     iq_str->issue_queue[k].src1_ready = true;
        //     //     iq_str->issue_queue[k].src2_ready = true;
        //     //     printf("srcs makred ready in issue queue\n");
        //     // }
        //     // if (iq_str->issue_queue[k].src1_tag == rob_buffer->buffer[current_inst.rob_tag].dst) {
        //     //     iq_str->issue_queue[k].src1_ready = true;
        //     // }
        //     // if (iq_str->issue_queue[k].src2_tag == rob_buffer->buffer[current_inst.rob_tag].dst) {
        //     //     iq_str->issue_queue[k].src2_ready = true;
        //     // }
        //     if (iq_str->issue_queue[k].valid){
        //     if (iq_str->issue_queue[k].src1_tag == current_inst.rob_tag) {
        //         iq_str->issue_queue[k].src1_ready = true;
        //     }
        //     if (iq_str->issue_queue[k].src2_tag == current_inst.rob_tag) {
        //         iq_str->issue_queue[k].src2_ready = true;
        //     }
        // }
        //     //printf("out of editing of iq\n");
        // }


        //printf("rob tag: %d\n", current_inst.rob_tag);
        //printf("going to next wb or exiting\n");
        RT->pipeline_instr.push_back(current_inst.seq_num); 
        RT->count++;
    }

    //printf("retire count in write back: %d\n", RT->count);
    WB->clear();
    //EX->count = (int)EX->execute_list.size();
    //RT->count = (int)RT->pipeline_instr.size();
    WB->count = (int)WB->pipeline_instr.size();
    return;
}

void Simulator::retire() {
    //if (!rob_buffer->buffer[rob_buffer->head].ready) return;
    if (RT->isEmpty()) return;
    int retired_inst_count = 0;
    // timing
    for (int i = 0; i < RT->pipeline_instr.size(); i++) {
        if (instr_list[RT->pipeline_instr[i]].RT.start == -1) {
            instr_list[RT->pipeline_instr[i]].RT.start = global_counter;
        }
    }

    // if (instr_list[RT->pipeline_instr[0]].RT.start == -1) {
    //     for (int i = 0; i < RT->pipeline_instr.size(); i++) {
    //         instr_list[RT->pipeline_instr[i]].RT.start = global_counter;
    //     }
    // }
    
    
    for (int i = 0; i < RT->pipeline_instr.size(); i++) {
        //printf("is retire ready: %d\n", instr_list[RT->pipeline_instr[i]].retired);
        //printf("retire count in retire: %d\n", RT->count);
        if (!instr_list[RT->pipeline_instr[i]].retired) {
            instr_list[RT->pipeline_instr[i]].RT.duration++;
        }
        //instr_list[RT->pipeline_instr[i]].RT.duration++;
    }
    int retired_seq = 0;
    
    //retire and clear rob index
    
    if (!rob_buffer->isEmpty()) {
        // for (int i = 0; i < RT->count && retired_inst_count < (int)params.width; i++) {
        //     if (RT->pipeline_instr)
        // }
        // rob_buffer->buffer[rob_buffer->head].global_idx == retired
        int &rob_head = rob_buffer->head;
        for (int i = 0; i < (int)params.width; i++) {//rob_buffer->rob_size; i++) {
            if (rob_buffer->buffer[rob_head].valid && rob_buffer->buffer[rob_head].ready) {
                if (instr_list[rob_buffer->buffer[rob_head].global_idx].retired) break;
                int rmt_index = rob_buffer->buffer[rob_head].dst;
                rob_buffer->buffer[rob_head].valid = false;
                // rob_buffer->buffer[rob_head].ready = false;
                instr_list[rob_buffer->buffer[rob_head].global_idx].retired = true;
                
                //std::vector <int> get_index = iq_str->oldest_up_to_width_indices(1);
                //rmt_index = get_index[0];
                int iq_index = iq_str->index_find(rob_buffer->buffer[rob_head].global_idx);
                if (iq_index != -1) {
                    iq_str->issue_queue[iq_index].valid = false;
                    iq_str->issue_queue[iq_index].src1_ready = false;
                    iq_str->issue_queue[iq_index].src2_ready = false;
                    iq_str->issue_queue[iq_index].global_idx = -1;
                    iq_str->issue_queue[iq_index].rob_tag = -1;
                    iq_str->issue_queue[iq_index].src1_tag = -1;
                    iq_str->issue_queue[iq_index].src2_tag = -1;
                    iq_str->count--;
                }
                instruction &current_inst = instr_list[rob_buffer->buffer[rob_head].global_idx];
                if (current_inst.dest != -1 && rmt[current_inst.dest].rob_tag == current_inst.rob_tag) {
                    rmt[current_inst.dest].rob_tag = -1;
                    rmt[current_inst.dest].valid = false;
                }

                // if (rob_buffer->buffer[rob_buffer->head].valid && rob_buffer->buffer[rob_buffer->head].ready) {
                //     int dest_reg = rob_buffer->buffer[rob_buffer->head].dst;
                //     if (dest_reg != -1) {
                //         rmt[dest_reg].valid = false;
                //         rmt[dest_reg].rob_tag = -1;
                //     }
                // }
                //RT->pipeline_instr.erase(RT->pipeline_instr.begin() + RT->pipeline_instr[rob_buffer->buffer[rob_buffer->head].global_idx]);
                rob_buffer->buffer[rob_head].dst = -1;
                retired_seq = rob_buffer->buffer[rob_head].global_idx;
                rob_buffer->buffer[rob_head].global_idx = -1;
                rob_buffer->head = (rob_head + 1) % (int)params.rob_size;
                rob_buffer->count--;
                auto rt_idx = std::find(RT->pipeline_instr.begin(), RT->pipeline_instr.end(), retired_seq);

                if (rt_idx != RT->pipeline_instr.end()) RT->pipeline_instr.erase(RT->pipeline_instr.begin() + std::distance(RT->pipeline_instr.begin(), rt_idx));
                RT->count--;
                retired_seq++;
                
            }
            else break;
        }
    }

    //for (int i = 0; i < )
    // for (int i = 0; i < retired; i++) {
    //     RT->pipeline_instr.erase(RT->pipeline_instr.begin() + 0);
    // }
    return;
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
    // printf("rob_size:%lu "
    //         "iq_size:%lu "
    //         "width:%lu "
    //         "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
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
    Simulator simulation(params, FP);
    //Pipeline_stage* DE = new Pipeline_stage(params.width);
    bool test = true;
    do {
        // global_counter++;
        //printf("FE empty: %d\n", sim.FE->isEmpty());
        simulation.retire();
        simulation.write_back();
        simulation.execute();
        //simulation.iq_func();
        simulation.issue();
        //simulation.iq_func();
        simulation.dispatch();
        simulation.RegRead();
        simulation.rename();
        simulation.decode();
        simulation.fetch();
        
        global_counter++;
        //if (global_counter >= 15) test = false;
        //printf("%d\n", instr_list[0].FE.start);
        //printf("global counter: %llx\n", global_counter);
        //test = false;
        test = simulation.advance_cycle();
        //if (global_counter >= 1000000) test = false;
    }
    while (test);

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
    printf("# === Simulator Command =========\n");
    printf("# ./sim %lu %lu %lu %s\n", params.rob_size, params.iq_size, params.width, trace_file);
    printf("# === Processor Configuration ===\n# ROB_SIZE = %lu\n# IQ_SIZE  = %lu\n# WIDTH    = %lu\n",params.rob_size, params.iq_size, params.width);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count    = %d\n", fetch_seq_counter);
    printf("# Cycles                       = %d\n", global_counter);
    printf("# Instructions Per Cycle (IPC) = %.2f\n", (double)fetch_seq_counter/global_counter);



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