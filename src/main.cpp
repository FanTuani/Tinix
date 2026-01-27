#include "kernel.h"
#include "shell/shell.h"

int main() {
    Kernel kernel; 
    Shell shell(kernel);
    shell.run();
    return 0;
}
