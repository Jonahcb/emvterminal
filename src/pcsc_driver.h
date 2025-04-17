#ifndef PCSC_DRIVER_H
#define PCSC_DRIVER_H

#include <stdint.h>
#include <stddef.h>

int init_reader(void);
int connect_to_card(void);
int transmit_apdu(const uint8_t *send_buf, size_t send_len,
                  uint8_t *recv_buf, size_t *recv_len);
void disconnect_card(void);
void close_reader(void);

#endif
