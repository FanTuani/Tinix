#pragma once
#include "fs/fs_defs.h"
#include "fs/inode_manager.h"
#include "fs/block_manager.h"
#include "fs/directory_manager.h"
#include "fs/file_descriptor_table.h"
#include "dev/disk.h"
#include <memory>
#include <string>

class FileSystem {
public:
    explicit FileSystem(DiskDevice* disk);
    ~FileSystem();

    bool format();
    bool mount();
    bool is_mounted() const { return mounted_; }

    // 目录操作
    bool create_directory(const std::string& path);
    bool list_directory(const std::string& path);
    std::string get_current_directory() const { return current_dir_; }
    bool change_directory(const std::string& path);
    
    // 文件操作
    bool create_file(const std::string& path);
    bool remove_file(const std::string& path);
    int open_file(const std::string& path);
    void close_file(int fd);
    ssize_t read_file(int fd, void* buffer, size_t size);
    ssize_t write_file(int fd, const void* buffer, size_t size);
    
    // 调试信息
    void print_superblock() const;
    void print_inode(uint32_t inode_num) const;

private:
    DiskDevice* disk_;
    SuperBlock superblock_;
    bool mounted_;
    std::string current_dir_;
    
    // 各功能模块
    std::unique_ptr<InodeManager> inode_mgr_;
    std::unique_ptr<BlockManager> block_mgr_;
    std::unique_ptr<DirectoryManager> dir_mgr_;
    std::unique_ptr<FileDescriptorTable> fd_table_;

    bool load_superblock();
    bool save_superblock();
    bool init_root_directory();
    void refresh_space_counters_from_bitmaps();
};
