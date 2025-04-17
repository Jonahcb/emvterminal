#include <stdio.h>
#include "pcsc_driver.h"
#include "emv_terminal.h"

int main(void) {
    if (!init_reader()) {
        printf("Failed to initialize reader.\n");
        return 1;
    }

    if (!connect_to_card()) {
        printf("Failed to connect to smartcard.\n");
        close_reader();
        return 1;
    }

    run_emv_transaction();  // SELECT AID, etc.

    disconnect_card();
    close_reader();

    return 0;
}
