#pragma once
#include "mem/memory_manager.h"
#include "proc/process_manager.h"
#include "dev/disk.h"

class Kernel {
public:
    Kernel(size_t num_frames, size_t page_size);
    
    ProcessManager& get_process_manager() { return pm_; }
    MemoryManager& get_memory_manager() { return mm_; }
    DiskDevice& get_disk_device() { return disk_; }
    
private:
    // 基础硬件设备
    DiskDevice disk_;

    // mm_ 必须在 pm_ 之前声明
    // 因为 ProcessManager 的构造函数需要 MemoryManager 引用
    MemoryManager mm_;
    ProcessManager pm_;
};
