#include "kernel.h"
#include "shell/shell.h"

int main() {
    Kernel kernel{16, 4096}; // 16 frames, 4KB page size
    Shell shell(kernel);
    shell.run();
    return 0;
}
