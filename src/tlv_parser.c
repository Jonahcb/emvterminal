#include "tlv_parser.h"
#include <stdio.h>

void parse_tlv(const uint8_t *buf, size_t len) {
    size_t i = 0;
    while (i < len) {
        uint8_t tag1 = buf[i++];
        uint8_t tag2 = 0;
        if ((tag1 & 0x1F) == 0x1F) tag2 = buf[i++];

        uint16_t tag = (tag2 == 0) ? tag1 : ((tag1 << 8) | tag2);

        uint8_t len_byte = buf[i++];
        size_t value_len = 0;
        if (len_byte & 0x80) {
            int num_bytes = len_byte & 0x7F;
            for (int j = 0; j < num_bytes; j++) value_len = (value_len << 8) | buf[i++];
        } else {
            value_len = len_byte;
        }

        printf("Tag %04X (%zu bytes): ", tag, value_len);
        for (size_t j = 0; j < value_len; j++) printf("%02X ", buf[i + j]);
        printf("\n");

        i += value_len;
    }
}

const uint8_t *find_tag(const uint8_t *buf, size_t len, uint16_t tag, size_t *out_len) {
    size_t i = 0;
    while (i < len) {
        uint8_t tag1 = buf[i++];
        uint8_t tag2 = 0;
        if ((tag1 & 0x1F) == 0x1F) tag2 = buf[i++];

        uint16_t current_tag = (tag2 == 0) ? tag1 : ((tag1 << 8) | tag2);

        uint8_t len_byte = buf[i++];
        size_t value_len = 0;
        if (len_byte & 0x80) {
            int num_bytes = len_byte & 0x7F;
            for (int j = 0; j < num_bytes; j++) value_len = (value_len << 8) | buf[i++];
        } else {
            value_len = len_byte;
        }

        if (current_tag == tag) {
            *out_len = value_len;
            return &buf[i];
        }

        i += value_len;
    }

    return NULL;
}
