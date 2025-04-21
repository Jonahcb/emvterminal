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
    if (!fci_template)
    {
        printf("No 6F FCI Template found.\n");
        return;
    }

    // Step 2: Extract A5 from inside 6F
    size_t a5_len;
    const uint8_t *a5_val = find_tag(fci_template, fci_len, 0xA5, &a5_len);
    if (!a5_val)
    {
        printf("No A5 tag found inside 6F.");
        return;
    }

    // Step 3: Extract 9F38 PDOL from inside A5
    size_t pdol_len;
    const uint8_t *pdol = find_tag(a5_val, a5_len, 0x9F38, &pdol_len);
    if (!pdol)
    {
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

    // Step 4: Read Record based on AFL = 94 tag from GPO
    size_t gpo_len;
    const uint8_t *gpo_template = find_tag(recv_buf, recv_len, 0x77, &gpo_len);
    if (!gpo_template)
    {
        printf("No 77 template found in GPO response.\n");
        return;
    }

    size_t afl_len;
    const uint8_t *afl = find_tag(gpo_template, gpo_len, 0x94, &afl_len);
    if (!afl || afl_len < 4)
    {
        printf("AFL (94) not found or too short.\n");
        return;
    }

    uint8_t sfi = afl[0] >> 3;
    uint8_t first_record = afl[1];

    uint8_t read_record_apdu[] = {
        0x00, 0xB2, first_record, (uint8_t)((sfi << 3) | 0x04), 0x00};

    recv_len = sizeof(recv_buf);
    if (!transmit_apdu(read_record_apdu, sizeof(read_record_apdu), recv_buf, &recv_len))
    {
        printf("READ RECORD failed.\n");
        return;
    }

    printf("\n-- READ RECORD Response --\n");
    parse_tlv(recv_buf, recv_len);

    // Step 5: Generate Application Cryptogram (GEN AC)
    uint8_t genac_apdu[] = {
        0x80, 0xAE, 0x00, 0x00, 0x0C, // CLA, INS, P1, P2, Lc (12 bytes)
        0x00, 0x00, 0x00, 0x00,       // Amount Authorized = 0
        0x00, 0x00, 0x00, 0x00,       // Amount Other = 0
        0x08, 0x40, 0x24, 0x04,       // Country Code = 0840, Date = 2024-04-xx
        0x00                          // Le
    };

    recv_len = sizeof(recv_buf);
    if (!transmit_apdu(genac_apdu, sizeof(genac_apdu), recv_buf, &recv_len))
    {
        printf("GEN AC failed.\n");
        return;
    }

    printf("\n-- GENERATE AC Response --\n");
    parse_tlv(recv_buf, recv_len);

    // Interpret result of GEN AC (Look for 9F27 tag: CID)
    size_t ac_len;
    const uint8_t *cid_ptr = find_tag(recv_buf, recv_len, 0x9F27, &ac_len);
    if (cid_ptr && ac_len == 1)
    {
        uint8_t cid = cid_ptr[0];
        printf("\n-- Cryptogram Information Data (CID): %02X --\n", cid);
        switch (cid)
        {
        case 0x00:
            printf("AAC: Transaction Declined by Card\n");
            break;
        case 0x40:
            printf("TC: Transaction Approved (Offline)\n");
            break;
        case 0x80:
            printf("ARQC: Go Online for Authorization\n");
            break;
        default:
            printf("Unknown or Proprietary CID\n");
            break;
        }
    }
    else
    {
        printf("\nCID (9F27) not found in GEN AC response.\n");
    }
}
