#pragma once
#include "mem/memory_manager.h"
#include "proc/process_manager.h"
#include "dev/disk.h"
#include "fs/file_system.h"

class Kernel {
public:
    Kernel();
    
    ProcessManager& get_process_manager() { return pm_; }
    MemoryManager& get_memory_manager() { return mm_; }
    DiskDevice& get_disk_device() { return disk_; }
    FileSystem& get_file_system() { return fs_; }
    
private:
    // 基础硬件设备
    DiskDevice disk_;
    
    // 文件系统
    FileSystem fs_;

    // mm_ 必须在 pm_ 之前声明
    // 因为 ProcessManager 的构造函数需要 MemoryManager 引用
    MemoryManager mm_;
    ProcessManager pm_;
};
