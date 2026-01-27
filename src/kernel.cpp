#include "kernel.h"

Kernel::Kernel() : disk_(), mm_(disk_), pm_(mm_) {}
