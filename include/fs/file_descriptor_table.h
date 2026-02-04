#pragma once
#include <map>
#include <cstdint>

struct OpenFile {
    uint32_t inode_num;
    uint32_t offset;
};

class FileDescriptorTable {
public:
    FileDescriptorTable();
    
    int alloc_fd(uint32_t inode_num);
    bool free_fd(int fd);
    
    OpenFile* get_open_file(int fd);
    
private:
    std::map<int, OpenFile> open_files_;
    int next_fd_;
};
