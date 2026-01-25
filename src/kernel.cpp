#include "kernel.h"

Kernel::Kernel(size_t num_frames, size_t page_size)
    : mm_(num_frames, page_size), pm_(mm_) {}
