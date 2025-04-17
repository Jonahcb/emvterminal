#ifndef APDU_BUILDER_H
#define APDU_BUILDER_H

#include <stdint.h>
#include <stddef.h>

int build_gpo_apdu(const uint8_t *pdol, size_t pdol_len, uint8_t *out_apdu, size_t *out_len);

#endif
