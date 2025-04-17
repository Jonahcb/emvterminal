#include "emv_terminal.h"
#include "pcsc_driver.h"
#include "tlv_parser.h"
#include "apdu_builder.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void run_emv_transaction(void)
{
    uint8_t select_apdu[] = {
        0x00, 0xA4, 0x04, 0x00, 0x07,
        0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10};

    uint8_t recv_buf[258];
    size_t recv_len = sizeof(recv_buf);

    if (!transmit_apdu(select_apdu, sizeof(select_apdu), recv_buf, &recv_len))
    {
        printf("Failed to SELECT AID.\n");
        return;
    }

    printf("\n-- SELECT AID Response --\n");
    parse_tlv(recv_buf, recv_len);


    // Step 1: Extract FCI Template (6F)
    size_t fci_len;
    const uint8_t *fci_template = find_tag(recv_buf, recv_len, 0x6F, &fci_len);
    if (!fci_template) {
        printf("No 6F FCI Template found.\n");
        return;
    }

    // Step 2: Extract A5 from inside 6F
    size_t a5_len;
    const uint8_t *a5_val = find_tag(fci_template, fci_len, 0xA5, &a5_len);
    if (!a5_val) {
        printf("No A5 tag found inside 6F.");
        return;
    }

    // Step 3: Extract 9F38 PDOL from inside A5
    size_t pdol_len;
    const uint8_t *pdol = find_tag(a5_val, a5_len, 0x9F38, &pdol_len);
    if (!pdol) {
        printf("No PDOL (9F38) found inside A5.\n");
        return;
    }

    uint8_t gpo_apdu[128];
    size_t gpo_apdu_len;

    if (!build_gpo_apdu(pdol, pdol_len, gpo_apdu, &gpo_apdu_len))
    {
        printf("Failed to build GPO APDU.\n");
        return;
    }

    recv_len = sizeof(recv_buf);
    if (!transmit_apdu(gpo_apdu, gpo_apdu_len, recv_buf, &recv_len))
    {
        printf("GPO failed.\n");
        return;
    }

    printf("\n-- GPO Response --\n");
    parse_tlv(recv_buf, recv_len);
}
