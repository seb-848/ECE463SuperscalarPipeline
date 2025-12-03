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

int IQ::valid_entries() {
        int count = 0;
        // for (int i = 0; i < iq_size; i++) {
        //     if ()
        //     if (issue_queue[i].valid && issue_queue[i].src1_ready && issue_queue[i].src2_ready) {
                
        //         count++;
        //     }
        // }

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
                //sorted_iq.push_back(issue_queue[i].global_idx);
            }
        }
        return count;
    }

std:: vector<int> IQ::oldest_up_to_width_indices(unsigned long int w) {
        std::vector <int> sorted_iq;
        //std::vector <basic_comp_entry> track;
        int valid = valid_entries();
        for (int i = 0; i < issue_queue.size(); i++) {
            if (issue_queue[i].valid) {
                if (instr_list[issue_queue[i].global_idx].src1 != -1) {
                    if (issue_queue[i].src1_ready) {
                        if (instr_list[issue_queue[i].global_idx].src2 != -1) {
                            if (issue_queue[i].src2_ready) {
                                sorted_iq.push_back(issue_queue[i].global_idx);
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
                        sorted_iq.push_back(issue_queue[i].global_idx);
                    }
                    else {
                        continue;
                    }
                }
                else {
                    sorted_iq.push_back(issue_queue[i].global_idx);
                    continue;
                }
                //sorted_iq.push_back(issue_queue[i].global_idx);
            }
        }

        std::sort(sorted_iq.begin(), sorted_iq.end());
        //printf("valid #: %d\n",valid);
        if (valid > (int)w) valid = (int)w;
        if (valid <= 0) return std::vector <int>();
        
        if ((int)sorted_iq.size() > valid) sorted_iq.resize(valid);
        //if (valid <= 0)
        // while (sorted_iq.size() > valid) {
        //     sorted_iq.pop_back();
        // }
        // for (int i = 0; i < sorted_iq.size(); i++) {
        //     printf("sorted iq after elim: %d, %d\n",i, sorted_iq[i]);
        // }
        //printf("sorted iq #: %d\n",sorted_iq.size());

        for (int i = 0; i < sorted_iq.size(); i++) {
            for (int k = 0; k < issue_queue.size(); k++) {
                if (sorted_iq[i] == issue_queue[k].global_idx) {
                    sorted_iq[i] = k;
                    break;
                }
            }
        }
        //printf("sorted iq #: %d\n",sorted_iq.size());

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
bool Simulator::advance_cycle() {
    bool advance = true;

    if (fetch_done && rob_buffer->isEmpty() && iq_str->isEmpty() && EX->isEmpty() && WB->isEmpty()) {
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
                //printf("start for %d : %d\n",i, instr.FE.start);
                FE->pipeline_instr.push_back(instr.seq_num); FE->count++;
                //FE->pipeline_instr[i] = instr.seq_num; FE->count++;
                //printf("FETCH: %d\n", FE->pipeline_instr[i]);

                // DE->pipeline_instr.push_back(instr.seq_num); DE->count++;
                // FE->clear();
                //printf("seq num: %d\n", instr_list[fetch_seq_counter - 1].seq_num);
            }
            else {
                fetch_done = true;
            }
        }
    }

    if (!DE->isEmpty()) {
        //FE->increment_duration(instr_list);
        for (int i = 0; i < FE->count; i++) {
            instr_list[FE->pipeline_instr[i]].FE.duration++;
        }
        return;
    }

    FE->fill_next_stage(DE);
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
    if (!RN->isEmpty()) return;
    DE->fill_next_stage(RN);
    return;
}

void Simulator::rename() {
    //printf("in rename\n");
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
    //printf("about to check for empty in regread and available space in rob buffer\n");
    if (!RR->isEmpty() || !rob_buffer->available(params.width)) return;
    for (int i = 0; i < RN->pipeline_instr.size(); i++) {
        instruction &current_inst = instr_list[RN->pipeline_instr[i]];
        
        // put into rob and get rob tag
        current_inst.rob_tag = rob_buffer->allocate(RN->pipeline_instr[i], current_inst.dest);

        // dont think we need the global sequential index
//printf("in rename loop\n");
        // getting value from arf
        //printf("rmt src1 check: %d\n",rmt[current_inst.src1].valid);
        if (current_inst.src1 == -1 || !rmt[current_inst.src1].valid) {
            current_inst.src1_tag = -1;
            current_inst.src1_ready = true;
        }
        else {
            //printf("THERE is rmt and it is valid\n");
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
        
        //printf("checking dest isnt seg\n");
        if (current_inst.dest != -1) {
            rmt[current_inst.dest].valid = true;
            rmt[current_inst.dest].rob_tag = current_inst.rob_tag;
        }
        

//printf("dest fixed\n");
        //RN->fill_next_stage(RR);
        //printf("next stage filled\n");
    }
    RN->fill_next_stage(RR);
    return;
}

void Simulator::RegRead() {
    //printf("in regread\n");
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
    return;
}

void Simulator::dispatch() {
    //printf("in dispatch\n");
    if (DI->isEmpty()) return;

    //timing
    if (instr_list[DI->pipeline_instr[0]].DI.start == -1) {
        for (int i = 0; i < DI->pipeline_instr.size(); i++) {
            instr_list[DI->pipeline_instr[i]].DI.start = global_counter;
        }
    }
    for (int i = 0; i < DI->pipeline_instr.size(); i++) {
        instr_list[DI->pipeline_instr[i]].DI.duration++;
    }


    if (!IS->isEmpty() || !iq_str->available(params.width)) return;
    //get available spot, use seq_num for queue function
    //std::vector<int> index_to_fill = iq_str->available_indices(params.width);
    // fill issue queue
    // printf("index to fill %d\n", index_to_fill.size());
    // for (int i = 0; i < index_to_fill.size(); i++) {
    //     printf("index to size: %d, %d\n",i, index_to_fill[i]);
    // }
    // for (int i = 0; i < index_to_fill.size(); i++) {
    //     instruction &current_inst = instr_list[DI->pipeline_instr[i]];
    //     iq_str->issue_queue[index_to_fill[i]].valid = true;
    //     iq_str->issue_queue[index_to_fill[i]].rob_tag = current_inst.rob_tag;
    //     iq_str->issue_queue[index_to_fill[i]].src1_ready = current_inst.src1_ready;
    //     iq_str->issue_queue[index_to_fill[i]].src1_tag = current_inst.src1_tag;
    //     iq_str->issue_queue[index_to_fill[i]].src2_ready = current_inst.src2_ready;
    //     iq_str->issue_queue[index_to_fill[i]].src2_tag = current_inst.src2_tag;
    //     iq_str->issue_queue[index_to_fill[i]].global_idx = DI->pipeline_instr[i];
    //     iq_str->count++;
    //     //printf("filling iq\n");
    // }
    int filling_index = 0;
    for (int i = 0; i < (int)DI->pipeline_instr.size(); i++) {
        instruction &current_inst = instr_list[DI->pipeline_instr[i]];
        for (int k = 0; k < (int)iq_str->issue_queue.size(); k++) {
            if (!iq_str->issue_queue[k].valid) {
                filling_index = k;
                break;
            }
        }
        iq_str->issue_queue[filling_index].valid = true;
        iq_str->issue_queue[filling_index].rob_tag = current_inst.rob_tag;
        iq_str->issue_queue[filling_index].src1_ready = current_inst.src1_ready;
        iq_str->issue_queue[filling_index].src1_tag = current_inst.src1_tag;
        iq_str->issue_queue[filling_index].src2_ready = current_inst.src2_ready;
        iq_str->issue_queue[filling_index].src2_tag = current_inst.src2_tag;
        iq_str->issue_queue[filling_index].global_idx = DI->pipeline_instr[i];
        iq_str->count++;
        //printf("filling iq\n");
    }
    // for (int i = 0; i < iq_str->issue_queue.size(); i++) {
    //     printf("%d: valid: %d\n",i, iq_str->issue_queue[i].valid);
    //     printf("%d: src1 ready: %d\n",i,iq_str->issue_queue[i].src1_ready);
    //     printf("%d: src2 ready: %d\n",i,iq_str->issue_queue[i].src2_ready);
    // }
    //DI->fill_next_stage(IS);
    //printf("clearing  dispatch\n");
    DI->clear();
    //delete[] index_to_fill;
    return;
}

void Simulator::issue() {
   // printf("in issue\n");
    if (iq_str->isEmpty()) return;
    //printf("ISSUE IS NOT EMPTY\n");
    //timing including issue queue
    for (int i = 0; i < params.iq_size; i++) {
        if (iq_str->issue_queue[i].valid) {
            if (instr_list[iq_str->issue_queue[i].global_idx].IS.start == -1) {
                instr_list[iq_str->issue_queue[i].global_idx].IS.start = global_counter;
            }
                instr_list[iq_str->issue_queue[i].global_idx].IS.duration++;
        }
    }
    
    //if (IS->isEmpty()) IS->pipeline_instr.resize(params.width);
    //printf("still in issue\n");
    //return;
    //printf("getting the indicies to take out of iq\n");

    //if (iq_str->valid_entries() == 0) return;

    //printf("valid entries were found\n");
    //printf("getting the indicies to take out of iq\n");
    std::vector <int> indicies = iq_str->oldest_up_to_width_indices(params.width);
    
    //for (int i = 0; i < )
    // }
    //printf("still printing from IS\n");
    //printf("IS count: %d, indices # of oldest: %d\n",IS->count, indicies[0]);
    
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

    // for (int i = 0; i < indicies.size(); i++) {

    // }

    //printf("done filling issue\n");
    //if (EX->space_available(IS->count)) return;
    // put IS into EX
    //printf("IS->COUNT: %d\n", IS->count);
    for (int i = 0; i < IS->count; i++) {
        //printf("ex is being filled\n");
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
        EX->count++;
        //stage->pipeline_instr
    }
    IS->count = 0;
    IS->clear();


    // for (int i = IS->count - EX->execute_list.size(); i < EX->execute_list.size(); i++) {
    //     if (EX->execute_list[i].)
    // }


    //IS->fill_next_stage(EX);
    //delete indicies;
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
        EX->execute_list[i].time_left--;
    }

    //std::vector <int> executed_inst;
    int execute_count = 0;

    //printf("ex is about to check to move to wb\n");
    // check for instructions done and add to WB
    for (int i = 0; i < EX->execute_list.size(); i++) {
        //printf("about to check for timing\n");
        if (EX->execute_list[i].time_left == 0) {
            //printf("checking for timing\n");
            WB->pipeline_instr.push_back(EX->execute_list[i].global_idx);
            WB->count++;//[WB->count++] = EX->execute_list[i].global_idx;
            //printf("transfered\n");
            //EX->execute_list.erase(EX->execute_list.begin() + i);
            //EX->count--;
            //executed_inst.push_back(i);
            //EX->execute_list[i].global_idx = -1;
            execute_count++;
            //printf("current loop done\n");
        }
    }

    //printf("transfer all done\n");
    

    // printf("global counter %d\n", global_counter);
    // for (int i = 0; i < execute_count; i++) {
    //     printf("EX contents i: %d, content: %d\n", i, EX->execute_list[i].global_idx);
    // }
    for (int i = execute_count; i > execute_count; i--) {
        if (EX->execute_list[i].time_left == 0) {
            EX->execute_list.erase(EX->execute_list.begin() + execute_count);
        }
        //EX->execute_list.erase(EX->execute_list.begin());
    }
    EX->count = EX->execute_list.size();
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
    for (int i = 0; i < WB->count; i++) {
        instruction &current_inst = instr_list[WB->pipeline_instr[i]];
        rob_buffer->buffer[current_inst.rob_tag].ready = true;
        //printf("about to enter the editing of issue queue\n");
        for (int k = 0; k < iq_str->iq_size; k++) {
            //printf("in editing of iq\n");
            //printf("iq seq: %d    current inst seq: %d\n",iq_str->issue_queue[k].global_idx, current_inst.seq_num);
            // if (iq_str->issue_queue[k].global_idx == current_inst.seq_num) {
            //     iq_str->issue_queue[k].src1_ready = true;
            //     iq_str->issue_queue[k].src2_ready = true;
            //     printf("srcs makred ready in issue queue\n");
            // }
            // if (iq_str->issue_queue[k].src1_tag == rob_buffer->buffer[current_inst.rob_tag].dst) {
            //     iq_str->issue_queue[k].src1_ready = true;
            // }
            // if (iq_str->issue_queue[k].src2_tag == rob_buffer->buffer[current_inst.rob_tag].dst) {
            //     iq_str->issue_queue[k].src2_ready = true;
            // }

            if (iq_str->issue_queue[k].src1_tag == current_inst.rob_tag) {
                iq_str->issue_queue[k].src1_ready = true;
            }
            if (iq_str->issue_queue[k].src2_tag == current_inst.rob_tag) {
                iq_str->issue_queue[k].src2_ready = true;
            }
            //printf("out of editing of iq\n");
        }
        //printf("rob tag: %d\n", current_inst.rob_tag);
        //printf("going to next wb or exiting\n");
        RT->pipeline_instr.push_back(current_inst.seq_num); 
        RT->count++;
    }

    //printf("retire count in write back: %d\n", RT->count);
    WB->clear();
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
    int retired = 0;
    
    //retire inst, remove from rob
    
    if (!rob_buffer->isEmpty()) {
        // for (int i = 0; i < RT->count && retired_inst_count < (int)params.width; i++) {
        //     if (RT->pipeline_instr)
        // }
        // rob_buffer->buffer[rob_buffer->head].global_idx == retired
        for (int i = 0; i < (int)params.width; i++) {//rob_buffer->rob_size; i++) {
            if (rob_buffer->buffer[rob_buffer->head].valid && rob_buffer->buffer[rob_buffer->head].ready) {
                if (instr_list[rob_buffer->buffer[rob_buffer->head].global_idx].retired) break;
                int rmt_index = rob_buffer->buffer[rob_buffer->head].dst;
                rob_buffer->buffer[rob_buffer->head].valid = false;
                rob_buffer->buffer[rob_buffer->head].ready = false;
                instr_list[rob_buffer->buffer[rob_buffer->head].global_idx].retired = true;
                
                //std::vector <int> get_index = iq_str->oldest_up_to_width_indices(1);
                //rmt_index = get_index[0];
                int iq_index = iq_str->index_find(rob_buffer->buffer[rob_buffer->head].global_idx);
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
                instruction &current_inst = instr_list[rob_buffer->buffer[rob_buffer->head].global_idx];
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
                rob_buffer->buffer[rob_buffer->head].dst = -1;
                rob_buffer->buffer[rob_buffer->head].global_idx = -1;
                rob_buffer->head = (rob_buffer->head + 1) % (int)params.rob_size;
                rob_buffer->count--;
                auto rt_idx = std::find(RT->pipeline_instr.begin(), RT->pipeline_instr.end(), retired);

                if (rt_idx != RT->pipeline_instr.end()) RT->pipeline_instr.erase(RT->pipeline_instr.begin() + std::distance(RT->pipeline_instr.begin(), rt_idx));
                RT->count--;
                retired++;
                
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
        simulation.issue();
        simulation.dispatch();
        simulation.RegRead();
        simulation.rename();
        simulation.decode();
        simulation.fetch();
        
        // if (!sim.DE->isEmpty()) printf("not empty\n");
        // else printf("empty\n");
        // printf("empty: %d\n", sim.DE->isEmpty());
        global_counter++;
        //if (global_counter >= 15) test = false;
        //printf("%d\n", instr_list[0].FE.start);
        //printf("global counter: %llx\n", global_counter);
        //test = false;
        test = simulation.advance_cycle();
        if (global_counter >= 100000) test = false;
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
