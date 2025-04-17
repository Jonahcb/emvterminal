#include "apdu_builder.h"
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
        memset(&pdol_data[pdol_data_len], 0x00, len);
        pdol_data_len += len;
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
    return 1;
}
