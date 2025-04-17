#ifndef TLV_PARSER_H
#define TLV_PARSER_H

#include <stdint.h>
#include <stddef.h>

void parse_tlv(const uint8_t *buf, size_t len);
const uint8_t *find_tag(const uint8_t *buf, size_t len, uint16_t tag, size_t *out_len);

#endif
