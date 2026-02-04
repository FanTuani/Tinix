#include "fs/block_manager.h"
#include <iostream>

BlockManager::BlockManager(DiskDevice* disk) 
    : disk_(disk), bitmap_dirty_(false) {
    inode_bitmap_.resize(BLOCK_SIZE);
    data_bitmap_.resize(BLOCK_SIZE);
}

bool BlockManager::load_bitmaps() {
    if (!disk_->read_block(INODE_BITMAP_BLOCK, inode_bitmap_.data())) {
        return false;
    }
    if (!disk_->read_block(DATA_BITMAP_BLOCK, data_bitmap_.data())) {
        return false;
    }
    return true;
}

bool BlockManager::save_bitmaps() {
    if (!disk_->write_block(INODE_BITMAP_BLOCK, inode_bitmap_.data())) {
        return false;
    }
    if (!disk_->write_block(DATA_BITMAP_BLOCK, data_bitmap_.data())) {
        return false;
    }
    bitmap_dirty_ = false;
    return true;
}

uint32_t BlockManager::alloc_inode() {
    uint32_t inode_num = find_free_bit(inode_bitmap_, MAX_INODES);
    if (inode_num == INVALID_INODE) {
        std::cerr << "[FS] No free inodes available" << std::endl;
        return INVALID_INODE;
    }
    
    set_bit(inode_bitmap_, inode_num);
    bitmap_dirty_ = true;
    return inode_num;
}

void BlockManager::free_inode(uint32_t inode_num) {
    clear_bit(inode_bitmap_, inode_num);
    bitmap_dirty_ = true;
}

uint32_t BlockManager::alloc_block() {
    uint32_t block_num = find_free_bit(data_bitmap_, MAX_DATA_BLOCKS);
    if (block_num == INVALID_BLOCK) {
        std::cerr << "[FS] No free blocks available" << std::endl;
        return INVALID_BLOCK;
    }
    
    set_bit(data_bitmap_, block_num);
    bitmap_dirty_ = true;
    
    uint32_t actual_block = DATA_BLOCKS_START + block_num;
    return actual_block;
}

void BlockManager::free_block(uint32_t block_num) {
    uint32_t bitmap_index = block_num - DATA_BLOCKS_START;
    clear_bit(data_bitmap_, bitmap_index);
    bitmap_dirty_ = true;
}

bool BlockManager::is_bit_set(const std::vector<uint8_t>& bitmap, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint32_t bit_offset = bit_index % 8;
    return (bitmap[byte_index] & (1 << bit_offset)) != 0;
}

void BlockManager::set_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint32_t bit_offset = bit_index % 8;
    bitmap[byte_index] |= (1 << bit_offset);
}

void BlockManager::clear_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint32_t bit_offset = bit_index % 8;
    bitmap[byte_index] &= ~(1 << bit_offset);
}

uint32_t BlockManager::find_free_bit(const std::vector<uint8_t>& bitmap, uint32_t max_bits) {
    for (uint32_t i = 0; i < max_bits; i++) {
        if (!is_bit_set(bitmap, i)) {
            return i;
        }
    }
    return INVALID_INODE;
}
