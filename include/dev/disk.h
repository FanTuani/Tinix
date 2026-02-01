#pragma once
#include "common/config.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class DiskDevice {
public:
    DiskDevice();
    ~DiskDevice();

    bool read_block(size_t block_id, uint8_t* out_buffer);
    bool write_block(size_t block_id, const uint8_t* in_buffer);

    size_t get_num_blocks() const { return num_blocks_; }
    size_t get_block_size() const { return block_size_; }

private:
    std::string filename_ = config::DISK_IMAGE_NAME;
    size_t num_blocks_ = config::DISK_NUM_BLOCKS;
    size_t block_size_ = config::DISK_BLOCK_SIZE;
    std::fstream disk_file_;

    void initialize_disk();
};
