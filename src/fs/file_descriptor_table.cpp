#include "fs/file_descriptor_table.h"

FileDescriptorTable::FileDescriptorTable() : next_fd_(3) {}

int FileDescriptorTable::alloc_fd(uint32_t inode_num) {
    int fd = next_fd_++;
    open_files_[fd] = {inode_num, 0};
    return fd;
}

bool FileDescriptorTable::free_fd(int fd) {
    return open_files_.erase(fd) > 0;
}

OpenFile* FileDescriptorTable::get_open_file(int fd) {
    auto it = open_files_.find(fd);
    if (it == open_files_.end()) {
        return nullptr;
    }
    return &it->second;
}
