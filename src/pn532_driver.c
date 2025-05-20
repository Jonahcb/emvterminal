#include <Wire.h>
#include <Adafruit_PN532.h>
#include "reader.h"            // your generic reader interface
#include "pn532_driver.h"      // implements reader_t pn532_driver
#include "emv_terminal.h"      // declares run_emv_transaction()

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

/// Initialize the PN532 hardware (1 on success, 0 on failure)
int init_reader(void) {
    // setup() already did nfc.begin() and nfc.SAMConfig()
    return (nfc.getFirmwareVersion() != 0) ? 1 : 0;
}

/// Block until an ISO14443A card is present (1 on success)
int connect_to_card(void) {
    uint8_t uid[7];
    uint8_t uidLen;
    bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A,
                                         uid, &uidLen);
    return found ? 1 : 0;
}

/// Send APDU (send_buf, send_len), receive into recv_buf (recv_len)
/// Returns 1 on success, 0 on failure
int transmit_apdu(const uint8_t *send_buf,
                  size_t send_len,
                  uint8_t *recv_buf,
                  size_t *recv_len)
{
    bool ok = nfc.inDataExchange((uint8_t*)send_buf,
                                 send_len,
                                 recv_buf,
                                 recv_len);
    return ok ? 1 : 0;
}

/// Tear down after a single card (no-op for PN532)
void disconnect_card(void) {
    // nothing to do
}

/// Close the reader completely (no-op for PN532)
void close_reader(void) {
    // nothing to do
}

/// The reader_t instance your EMV library will use
reader_t pn532_driver = {
    .init         = init_reader,
    .connect_card = connect_to_card,
    .transmit     = transmit_apdu,
    .disconnect   = disconnect_card,
    .close        = close_reader
};

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();

  // ——— Initialize our EMV “reader” ———
  // point the common EMV code at this PN532 driver:
  current_reader = &pn532_driver;
  if (! current_reader->init()) {
    Serial.println("EMV reader init failed");
    while (1);
  }

  Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);

    if (uidLength == 4) {
      // We probably have a Mifare Classic card ...
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];
      cardid <<= 8;
      cardid |= uid[3];
      Serial.print("Seems to be a Mifare Classic card #");
      Serial.println(cardid);
    }
    delay(2000);


    // ——— Drive the full EMV transaction flow ———
    Serial.println("⏳ Running EMV transaction …");
    run_emv_transaction();
    Serial.println("✅ EMV transaction complete.");


  }
}