#pragma once
#include "fs/fs_defs.h"
#include "dev/disk.h"
#include <vector>
#include <cstdint>

class BlockManager {
public:
    explicit BlockManager(DiskDevice* disk);
    
    bool load_bitmaps();
    bool save_bitmaps();
    
    uint32_t alloc_inode();
    void free_inode(uint32_t inode_num);
    
    uint32_t alloc_block();
    void free_block(uint32_t block_num);
    
    bool is_bitmap_dirty() const { return bitmap_dirty_; }
    void set_bitmap_dirty(bool dirty) { bitmap_dirty_ = dirty; }
    
private:
    DiskDevice* disk_;
    std::vector<uint8_t> inode_bitmap_;
    std::vector<uint8_t> data_bitmap_;
    bool bitmap_dirty_;
    
    bool is_bit_set(const std::vector<uint8_t>& bitmap, uint32_t bit_index);
    void set_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index);
    void clear_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index);
    uint32_t find_free_bit(const std::vector<uint8_t>& bitmap, uint32_t max_bits);
};
