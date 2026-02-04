#pragma once
#include "fs/fs_defs.h"
#include "fs/inode_manager.h"
#include "fs/block_manager.h"
#include <string>

class DirectoryManager {
public:
    DirectoryManager(DiskDevice* disk, InodeManager* inode_mgr, BlockManager* block_mgr);
    
    uint32_t lookup_path(const std::string& path, const std::string& current_dir);
    uint32_t lookup_in_directory(uint32_t dir_inode, const std::string& name);
    
    bool add_directory_entry(uint32_t dir_inode, const std::string& name, uint32_t inode_num);
    bool remove_directory_entry(uint32_t dir_inode, const std::string& name);
    
    bool create_directory(const std::string& path, const std::string& current_dir);
    bool list_directory(const std::string& path, const std::string& current_dir);
    
    std::string normalize_path(const std::string& path, const std::string& current_dir);
    void split_path(const std::string& path, std::string& parent, std::string& name);
    
private:
    DiskDevice* disk_;
    InodeManager* inode_mgr_;
    BlockManager* block_mgr_;
};
