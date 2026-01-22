#pragma once
#include "process.h"
#include <map>
#include <queue>

class ProcessManager {
public:
    int create_process(int total_time = 10);
    void terminate_process(int pid);
    void dump_processes() const;
    void tick();
    void run_process(int pid);
    void block_process(int pid, int duration);
    void wakeup_process(int pid);

private:
    std::map<int, PCB> processes_;
    std::queue<int> ready_queue_;
    int next_pid_ = 1;
    int next_tick_ = 0;
    int current_running_ = -1;
    
    void schedule();
    void check_blocked_processes();
};
