#include "proc/process_manager.h"
#include "shell/shell.h"

int main() {
    ProcessManager pm;
    Shell shell(pm);
    shell.run();
    return 0;
}
