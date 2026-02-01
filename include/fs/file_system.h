#pragma once
#include "fs/fs_defs.h"
#include "dev/disk.h"
#include <memory>
#include <vector>
#include <string>

class FileSystem {
public:
    explicit FileSystem(DiskDevice* disk);
    ~FileSystem();

    // 格式化文件系统
    bool format();
    
    // 挂载文件系统
    bool mount();
    
    // 检查文件系统是否已挂载
    bool is_mounted() const { return mounted_; }

    // Inode 管理
    uint32_t alloc_inode();
    void free_inode(uint32_t inode_num);
    bool read_inode(uint32_t inode_num, Inode& out_inode);
    bool write_inode(uint32_t inode_num, const Inode& inode);

    // 数据块管理
    uint32_t alloc_block();
    void free_block(uint32_t block_num);

    // SuperBlock 访问
    const SuperBlock& get_superblock() const { return superblock_; }
    
    // 调试信息
    void print_superblock() const;
    void print_inode(uint32_t inode_num) const;

private:
    DiskDevice* disk_;
    SuperBlock superblock_;
    bool mounted_;
    
    // 位图缓存
    std::vector<uint8_t> inode_bitmap_;
    std::vector<uint8_t> data_bitmap_;
    bool bitmap_dirty_;

    // 辅助方法
    bool load_superblock();
    bool save_superblock();
    bool load_bitmaps();
    bool save_bitmaps();
    
    bool is_bit_set(const std::vector<uint8_t>& bitmap, uint32_t bit_index);
    void set_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index);
    void clear_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index);
    
    uint32_t find_free_bit(const std::vector<uint8_t>& bitmap, uint32_t max_bits);
    
    // 初始化根目录
    bool init_root_directory();
};
