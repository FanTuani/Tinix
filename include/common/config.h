#pragma once
#include <cstddef>

namespace tinix {
namespace config {

// mem
constexpr size_t PAGE_FRAMES = 8;              // 物理内存页框数
constexpr size_t PAGE_SIZE = 4096;             // 4 KB
constexpr size_t DEFAULT_VIRTUAL_PAGES = 256;  // 每个进程虚拟空间大小

// disk
constexpr const char* DISK_IMAGE_NAME = "disk.img";
constexpr size_t DISK_BLOCK_SIZE = 4096;  // 块大小
constexpr size_t DISK_NUM_BLOCKS = 1024;  // 总块数

// proc
constexpr int DEFAULT_TIME_SLICE = 3;  // 时间片长度

}
}
