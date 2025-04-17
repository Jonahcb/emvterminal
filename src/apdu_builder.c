#include "emv_terminal.h"
#include "pcsc_driver.h"
#include "tlv_parser.h"
#include "apdu_builder.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int build_gpo_apdu(const uint8_t *pdol, size_t pdol_len, uint8_t *out_apdu, size_t *out_len) {
    uint8_t apdu[] = { 0x80, 0xA8, 0x00, 0x00 };
    uint8_t pdol_data[64];
    size_t pdol_data_len = 0;

    size_t i = 0;
    while (i < pdol_len) {
        uint8_t tag1 = pdol[i++];
        uint8_t tag2 = 0;
        if ((tag1 & 0x1F) == 0x1F) tag2 = pdol[i++];
        uint8_t len = pdol[i++];

        // Fill with meaningful defaults based on tag
        if ((tag1 == 0x9F && tag2 == 0x66) && len == 4) {
            // Terminal Transaction Qualifiers
            pdol_data[pdol_data_len++] = 0xE0;
            pdol_data[pdol_data_len++] = 0x00;
            pdol_data[pdol_data_len++] = 0xC8;
            pdol_data[pdol_data_len++] = 0x00;
        } else if ((tag1 == 0x9F && tag2 == 0x02) && len == 6) {
            // Amount Authorized
            memset(&pdol_data[pdol_data_len], 0x00, 5);
            pdol_data[pdol_data_len + 5] = 0x01; // 1 cent
            pdol_data_len += 6;
        } else if ((tag1 == 0x9F && tag2 == 0x03) && len == 6) {
            // Amount Other
            memset(&pdol_data[pdol_data_len], 0x00, 6);
            pdol_data_len += 6;
        } else if ((tag1 == 0x9F && tag2 == 0x1A) && len == 2) {
            // Terminal Country Code (USA = 0840)
            pdol_data[pdol_data_len++] = 0x08;
            pdol_data[pdol_data_len++] = 0x40;
        } else if (tag1 == 0x95 && len == 5) {
            // Terminal Verification Results
            memset(&pdol_data[pdol_data_len], 0x00, 5);
            pdol_data_len += 5;
        } else if ((tag1 == 0x5F && tag2 == 0x2A) && len == 2) {
            // Transaction Currency Code (USD = 0840)
            pdol_data[pdol_data_len++] = 0x08;
            pdol_data[pdol_data_len++] = 0x40;
        } else if ((tag1 == 0x9A) && len == 3) {
            // Transaction Date (YYMMDD)
            pdol_data[pdol_data_len++] = 0x24; // 2024
            pdol_data[pdol_data_len++] = 0x04; // April
            pdol_data[pdol_data_len++] = 0x17; // 17th
        } else if ((tag1 == 0x9C) && len == 1) {
            // Transaction Type (00 = purchase)
            pdol_data[pdol_data_len++] = 0x00;
        } else if ((tag1 == 0x9F && tag2 == 0x37) && len == 4) {
            // Unpredictable Number
            pdol_data[pdol_data_len++] = 0x12;
            pdol_data[pdol_data_len++] = 0x34;
            pdol_data[pdol_data_len++] = 0x56;
            pdol_data[pdol_data_len++] = 0x78;
        } else {
            // Default: fill with zeros
            memset(&pdol_data[pdol_data_len], 0x00, len);
            pdol_data_len += len;
        }
    }

    uint8_t cmd_data[70];
    size_t cmd_data_len = 0;
    cmd_data[cmd_data_len++] = 0x83;
    cmd_data[cmd_data_len++] = pdol_data_len;
    memcpy(&cmd_data[cmd_data_len], pdol_data, pdol_data_len);
    cmd_data_len += pdol_data_len;

    size_t offset = 0;
    memcpy(&out_apdu[offset], apdu, sizeof(apdu));
    offset += sizeof(apdu);
    out_apdu[offset++] = (uint8_t)cmd_data_len;
    memcpy(&out_apdu[offset], cmd_data, cmd_data_len);
    offset += cmd_data_len;
    out_apdu[offset++] = 0x00;

    *out_len = offset;

    // Print full GPO APDU message
    printf("\n-- GPO APDU to be sent (%zu bytes) --\n", *out_len);
    for (size_t j = 0; j < *out_len; j++) {
        printf("%02X ", out_apdu[j]);
    }
    printf("\n");

    return 1;
}
