#ifndef PN532_DRIVER_H
#define PN532_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <stdint.h>
#include <stddef.h>
#include "reader.h"     


/// Initialize the PN532 hardware (returns 1 on success)
int init_reader(void);

/// Block until a card is present (returns 1 on success)
int connect_to_card(void);

/// Send an APDU (send_buf, send_len) and receive into recv_buf (recv_len)
int transmit_apdu(const uint8_t *send_buf,
                  size_t send_len,
                  uint8_t *recv_buf,
                  size_t *recv_len);

/// Disconnect / clean up after a single card
void disconnect_card(void);

/// Close down the reader completely (optional)
void close_reader(void);

// —— The reader_t instance your EMV library will bind to ——
/// Defined in pn532_driver.cpp (or at bottom of your sketch)
extern reader_t pn532_driver;

#endif  // PN532_DRIVER_H
