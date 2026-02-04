#pragma once
#include "fs/fs_defs.h"
#include "dev/disk.h"
#include <cstdint>

class InodeManager {
public:
    explicit InodeManager(DiskDevice* disk);
    
    bool read_inode(uint32_t inode_num, Inode& out_inode);
    bool write_inode(uint32_t inode_num, const Inode& inode);
    
private:
    DiskDevice* disk_;
};
