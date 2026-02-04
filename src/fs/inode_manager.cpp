#include "fs/inode_manager.h"
#include <cstring>
#include <vector>

InodeManager::InodeManager(DiskDevice* disk) : disk_(disk) {}

bool InodeManager::read_inode(uint32_t inode_num, Inode& out_inode) {
    uint32_t inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    uint32_t block_num = INODE_TABLE_START + inode_num / inodes_per_block;
    uint32_t offset = (inode_num % inodes_per_block) * sizeof(Inode);
    
    std::vector<uint8_t> block_data(BLOCK_SIZE);
    if (!disk_->read_block(block_num, block_data.data())) {
        return false;
    }
    
    memcpy(&out_inode, block_data.data() + offset, sizeof(Inode));
    return true;
}

bool InodeManager::write_inode(uint32_t inode_num, const Inode& inode) {
    uint32_t inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    uint32_t block_num = INODE_TABLE_START + inode_num / inodes_per_block;
    uint32_t offset = (inode_num % inodes_per_block) * sizeof(Inode);
    
    std::vector<uint8_t> block_data(BLOCK_SIZE);
    if (!disk_->read_block(block_num, block_data.data())) {
        return false;
    }
    
    memcpy(block_data.data() + offset, &inode, sizeof(Inode));
    
    if (!disk_->write_block(block_num, block_data.data())) {
        return false;
    }
    
    return true;
}
