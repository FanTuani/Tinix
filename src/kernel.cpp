#include "kernel.h"
#include "common/config.h"

Kernel::Kernel(size_t num_frames, size_t page_size)
    : disk_(tinix::config::DISK_IMAGE_NAME, tinix::config::DISK_NUM_BLOCKS, page_size), 
      mm_(num_frames, page_size, disk_), 
      pm_(mm_) {}
