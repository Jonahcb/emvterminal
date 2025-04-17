#include "pcsc_driver.h"
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#include <stdio.h>
#include <string.h>

static SCARDCONTEXT hContext;
static SCARDHANDLE hCard;
static DWORD dwActiveProtocol;
static char reader_name[256];

int init_reader(void) {
    LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    if (rv != SCARD_S_SUCCESS) {
        printf("SCardEstablishContext failed: %s\n", pcsc_stringify_error(rv));
        return 0;
    }

    DWORD dwReaders = sizeof(reader_name);
    rv = SCardListReaders(hContext, NULL, reader_name, &dwReaders);
    if (rv != SCARD_S_SUCCESS) {
        printf("SCardListReaders failed: %s\n", pcsc_stringify_error(rv));
        return 0;
    }

    printf("Using reader: %s\n", reader_name);
    return 1;
}

int connect_to_card(void) {
    LONG rv = SCardConnect(hContext, reader_name, SCARD_SHARE_SHARED,
                           SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                           &hCard, &dwActiveProtocol);
    if (rv != SCARD_S_SUCCESS) {
        printf("SCardConnect failed: %s\n", pcsc_stringify_error(rv));
        return 0;
    }
    return 1;
}

int transmit_apdu(const uint8_t *send_buf, size_t send_len,
                  uint8_t *recv_buf, size_t *recv_len) {
    const SCARD_IO_REQUEST *pioSendPci = (dwActiveProtocol == SCARD_PROTOCOL_T1)
                                             ? SCARD_PCI_T1
                                             : SCARD_PCI_T0;
    LONG rv = SCardTransmit(hCard, pioSendPci, send_buf, send_len, NULL, recv_buf, (DWORD *)recv_len);
    if (rv != SCARD_S_SUCCESS) {
        printf("SCardTransmit failed: %s\n", pcsc_stringify_error(rv));
        return 0;
    }
    return 1;
}

void disconnect_card(void) {
    SCardDisconnect(hCard, SCARD_LEAVE_CARD);
}

void close_reader(void) {
    SCardReleaseContext(hContext);
}
