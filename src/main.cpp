#include "kernel.h"
#include "shell/shell.h"
#include "common/config.h"

int main() {
    Kernel kernel{tinix::config::PAGE_FRAMES, tinix::config::PAGE_SIZE}; 
    Shell shell(kernel);
    shell.run();
    return 0;
}
