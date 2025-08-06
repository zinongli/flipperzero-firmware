#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>
#include <nfc/protocols/iso15693_3/iso15693_3.h>
#include <bit_lib.h>
#include <mbedtls/des.h>

#define TAG "Konami"

static const uint8_t konami_3des_key[24] = {0x7E, 0x92, 0x4E, 0xD8, 0xD8, 0x84, 0x64, 0xC6,
                                            0x5C, 0xB2, 0xDE, 0xEA, 0xB0, 0xB0, 0xB0, 0xCA,
                                            0x9A, 0xCA, 0x90, 0xC2, 0xB2, 0xE0, 0xF2, 0x42};

static const char* alphabet[32] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A",
                                   "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M",
                                   "N", "P", "R", "S", "T", "U", "W", "X", "Y", "Z"};

static void encrypt_3des(uint8_t* kid, uint8_t* encrypted) {
    mbedtls_des3_context des3_ctx;
    mbedtls_des3_init(&des3_ctx);
    mbedtls_des3_set3key_enc(&des3_ctx, konami_3des_key);
    uint8_t iv[8] = {0};
    mbedtls_des3_crypt_cbc(
        &des3_ctx, MBEDTLS_DES_ENCRYPT, ISO15693_3_UID_SIZE, iv, kid, encrypted);
    mbedtls_des3_free(&des3_ctx);
}

static void unpack5(const uint8_t* in, uint8_t* out) {
    uint8_t padded[9] = {0};
    memcpy(padded, in, 8);
    padded[8] = 0x00; // Padding byte

    for(size_t i = 0; i < 13; i++) {
        out[i] = bit_lib_get_bits(padded, i * 5, 5);
    }
}

static uint8_t checksum(const uint8_t* data) {
    uint32_t chk = 0;
    for(size_t i = 0; i < 15; i++) {
        chk += (uint32_t)data[i] * (uint32_t)(i % 3 + 1);
    }

    while(chk > 31) {
        chk = (chk >> 5) + (chk & 31);
    }

    return (uint8_t)chk;
}

static bool konami_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const Iso15693_3Data* data = nfc_device_get_data(device, NfcProtocolIso15693_3);

    bool parsed = false;

    do {
        uint8_t card_type = 0;
        if(data->uid[0] == 0xE0 && data->uid[1] == 0x04) {
            card_type = 1;
        } else if(data->uid[0] == 0) {
            card_type = 2;
        } else {
            FURI_LOG_D(TAG, "Invalid UID prefix");
            break;
        }

        uint8_t kid[ISO15693_3_UID_SIZE];
        for(size_t i = 0; i < ISO15693_3_UID_SIZE; i++) {
            kid[i] = data->uid[ISO15693_3_UID_SIZE - 1 - i]; // Reversed UID
        }

        uint8_t encrypted[ISO15693_3_UID_SIZE];
        encrypt_3des(kid, encrypted);

        uint8_t unpacked[16] = {0};
        unpack5(encrypted, unpacked);

        unpacked[0] ^= card_type;
        unpacked[13] = 1;
        for(size_t i = 1; i < 14; i++) {
            unpacked[i] ^= unpacked[i - 1];
        }
        unpacked[14] = card_type;
        unpacked[15] = checksum(unpacked);

        furi_string_cat_str(parsed_data, "\e#e-Amusement\n");
        furi_string_cat_str(parsed_data, "KONAMI ID:\n");

        for(size_t i = 0; i < 16; i++) {
            furi_string_cat_printf(parsed_data, "%s", alphabet[unpacked[i] & 0x1F]);
            switch(i) {
            case 3:
            case 7:
            case 11:
                furi_string_cat_printf(parsed_data, " ");
                break;
            }
        }

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin konami_plugin = {
    .protocol = NfcProtocolIso15693_3,
    .verify = NULL,
    .read = NULL,
    .parse = konami_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor konami_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &konami_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* konami_plugin_ep(void) {
    return &konami_plugin_descriptor;
}
