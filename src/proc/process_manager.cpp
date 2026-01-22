#include "proc/process_manager.h"
#include <iostream>

int ProcessManager::create_process(int total_time) {
    int pid = next_pid_++;
    processes_[pid] =
        PCB{.pid = pid, .state = ProcessState::Ready, .total_time = total_time};
    ready_queue_.push(pid);
    std::cout << "Process " << pid << " created (total_time=" << total_time
              << ") and added to ready queue\n";
    return pid;
}

void ProcessManager::terminate_process(int pid) {
    if (processes_.find(pid) == processes_.end()) {
        std::cout << "Process " << pid << " not found.\n";
        return;
    }
    processes_.erase(pid);
    if (pid == current_running_) {
        current_running_ = -1;
    }
    std::cout << "Process " << pid << " terminated.\n";
}

void ProcessManager::dump_processes() const {
    std::cout << "PID\tState\t\tRemain\tCPU/Total\tBlocked\n";
    for (const auto& [pid, pcb] : processes_) {
        std::string state_str;
        switch (pcb.state) {
            case ProcessState::New:
                state_str = "New";
                break;
            case ProcessState::Ready:
                state_str = "Ready";
                break;
            case ProcessState::Running:
                state_str = "Running";
                break;
            case ProcessState::Blocked:
                state_str = "Blocked";
                break;
            case ProcessState::Terminated:
                state_str = "Terminated";
                break;
        }
        std::cout << pid << "\t" << state_str << "\t\t" << pcb.remaining_time
                  << "\t" << pcb.cpu_time << "/" << pcb.total_time << "\t\t"
                  << pcb.blocked_time << "\n";
    }
    if (current_running_ != -1) {
        std::cout << "Currently running: " << current_running_ << "\n";
    } else {
        std::cout << "CPU idle\n";
    }
}

void ProcessManager::tick() {
    std::cout << "=== Tick " << next_tick_++ << " ===\n";
    check_blocked_processes();
    if (current_running_ != -1) {
        // 运行 1 tick
        auto& pcb = processes_[current_running_];
        pcb.remaining_time--;
        pcb.cpu_time++;
        std::cout << "[Tick] Process " << current_running_ << " executing ("
                  << pcb.cpu_time << "/" << pcb.total_time
                  << ", remaining: " << pcb.remaining_time << ")\n";

        if (pcb.cpu_time >= pcb.total_time) {  // 运行结束
            std::cout << "[Tick] Process " << current_running_
                      << " completed\n";
            pcb.state = ProcessState::Terminated;
            processes_.erase(current_running_);
            current_running_ = -1;
        } else if (pcb.remaining_time <= 0) {  // 时间片完
            std::cout << "[Tick] Process " << current_running_
                      << " time slice exhausted\n";
            pcb.state = ProcessState::Ready;
            pcb.remaining_time = pcb.time_slice;
            ready_queue_.push(pcb.pid);
            current_running_ = -1;
        }
    }
    if (current_running_ == -1) {  // CPU 空闲，触发调度
        schedule();
    }
}

void ProcessManager::schedule() {
    while (ready_queue_.size()) {
        int pid = ready_queue_.front();
        ready_queue_.pop();
        if (processes_.find(pid) == processes_.end()) {
            continue;  // 就绪队列可能存在已被终止的非法进程
        }
        auto& pcb = processes_[pid];
        if (pcb.state != ProcessState::Ready) {
            continue;  // 进程未就绪
        }
        // 调度进程开始运行
        current_running_ = pcb.pid;
        pcb.state = ProcessState::Running;
        std::cout << "[Schedule] Process " << pid << " is now running\n";
        return;
    }
    // 未能调度任何进程
    std::cout << "[Schedule] CPU idle - no ready processes\n";
}

void ProcessManager::run_process(int pid) {
    if (processes_.find(pid) == processes_.end()) {
        std::cout << "Process " << pid << " not found.\n";
        return;
    }
    if (processes_[pid].state != ProcessState::Ready) {
        std::cout << "Process " << pid << " is not in Ready state\n";
        return;
    }

    if (current_running_ != -1) {  // 抢占，改变当前进程状态
        processes_[current_running_].state = ProcessState::Ready;
        ready_queue_.push(current_running_);
        std::cout << "Process " << current_running_ << " preempted\n";
    }

    auto& pcb = processes_[pid];
    current_running_ = pid;
    pcb.state = ProcessState::Running;
    std::cout << "Process " << pid << " is now running\n";
}

void ProcessManager::block_process(int pid, int duration) {
    if (processes_.find(pid) == processes_.end()) {
        std::cout << "Process " << pid << " not found.\n";
        return;
    }
    auto& pcb = processes_[pid];
    if (pcb.state != ProcessState::Running &&
        pcb.state != ProcessState::Ready) {
        std::cout << "Process " << pid
                  << " cannot be blocked in its current state\n";
        return;
    }

    pcb.state = ProcessState::Blocked;
    pcb.blocked_time = duration;
    std::cout << "Process " << pid << " is blocked for " << duration
              << " ticks\n";

    if (pid == current_running_) {  // 当前进程被阻塞，触发调度
        current_running_ = -1;
        schedule();
    }
    // 就绪队列中可能存在该进程的冗余项，暂不移除
}

void ProcessManager::wakeup_process(int pid) {
    if (processes_.find(pid) == processes_.end()) {
        std::cout << "Process " << pid << " not found.\n";
        return;
    }
    auto& pcb = processes_[pid];
    if (pcb.state != ProcessState::Blocked) {
        std::cout << "Process " << pid << " is not blocked\n";
        return;
    }

    pcb.state = ProcessState::Ready;
    pcb.blocked_time = 0;
    ready_queue_.push(pid);
    std::cout << "Process " << pid << " woken up and added to ready queue\n";
}

void ProcessManager::check_blocked_processes() {
    // 更新阻塞进程的阻塞时间
    for (auto& [pid, pcb] : processes_) {
        if (pcb.state == ProcessState::Blocked && pcb.blocked_time > 0) {
            pcb.blocked_time--;
            if (pcb.blocked_time <= 0) {
                pcb.state = ProcessState::Ready;
                ready_queue_.push(pid);
                std::cout << "[Tick] Process " << pid << " auto-woken up\n";
            }
        }
    }
}