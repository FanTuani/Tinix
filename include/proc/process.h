enum class ProcessState { New, Ready, Running, Blocked, Terminated };

struct PCB {
    int pid = -1;
    ProcessState state = ProcessState::New;
    int time_slice = 3;
    int remaining_time = 3;
    int cpu_time = 0;
    int total_time = 10;
    int blocked_time = 0;
};