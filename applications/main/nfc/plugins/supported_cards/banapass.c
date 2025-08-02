#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Banapass"

static const uint64_t banapass_key = 0x6090D00632F5;
static const uint64_t banapass_key_b_value_block = 0x019761AA8082;
static const uint64_t banapass_key_b_access_code = 0x574343467632;

static bool banapass_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 0);
        uint64_t key_a = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        uint64_t key_b = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key_a != banapass_key) break;

        furi_string_set_str(parsed_data, "\e#Banapass\n");
        furi_string_cat_str(
            parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        furi_string_cat_str(parsed_data, "\nBandai Namco Passport\n");

        // banapass Magic is stored at block 1, byte 2-7
        uint8_t magic_bytes[6];
        for(int i = 0; i < 6; i++) {
            magic_bytes[i] = data->block[1].data[2 + i];
        }

        // verify banapass magic
        if(magic_bytes[0] != 'N' || magic_bytes[1] != 'B' || magic_bytes[2] != 'G' ||
           magic_bytes[3] != 'I' || magic_bytes[4] != 'C')
            break;

        // banapass checksum is stored at block 1, starts from byte 8-15
        uint8_t check_sum[8];
        for(int i = 0; i < 8; i++) {
            check_sum[i] = data->block[1].data[8 + i];
        }

        furi_string_cat_str(
            parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        switch(key_b) {
        case banapass_key_b_value_block:
            int32_t value = 0;
            uint8_t addr = 0;
            bool value_found = mf_classic_block_to_value(
                &data->block[2], &value, &addr); // block 2 is value block
            if(value_found) {
                furi_string_cat_printf(parsed_data, "\nValue: %ld", value);
            }
            furi_string_cat_str(
                parsed_data,
                "\nPlease check the back of\nyour Bandai Namco Passport\nfor the Access Code\n");
            furi_string_cat_str(
                parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
            break;

        case banapass_key_b_access_code:
            // banapass access code is stored as decimal hex representation in block 2, starts from byte 6, len 10 bytes
            uint8_t access_code[10];

            furi_string_cat_printf(parsed_data, "\nAccess Code:\n");
            bool access_code_is_bcd = true;

            for(int i = 0; i < 10; i++) {
                access_code[i] = data->block[2].data[6 + i];
                furi_string_cat_printf(parsed_data, "%02X", access_code[i]);
                if(i % 2 == 1) {
                    furi_string_cat_str(parsed_data, " ");
                }
                if(access_code[i] > 9) access_code_is_bcd = false;
            }
            furi_string_cat_printf(
                parsed_data, "\nBCD valid: %s\n", access_code_is_bcd ? "Yes" : "No");
            furi_string_cat_str(
                parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
            break;
        default:
            break;
        }

        furi_string_cat_str(parsed_data, "\nMagic:\n");
        for(int i = 0; i < 6; i++) {
            furi_string_cat_printf(parsed_data, "%c", magic_bytes[i]);
        }
        furi_string_cat_str(parsed_data, "\nChecksum:\n");
        for(int i = 0; i < 8; i++) {
            furi_string_cat_printf(parsed_data, "%02X ", check_sum[i]);
        }

        furi_string_cat_str(parsed_data, "\n");
        furi_string_cat_str(
            parsed_data, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");

        parsed = true;

    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin banapass_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,
    .read = NULL,
    .parse = banapass_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor banapass_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &banapass_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* banapass_plugin_ep(void) {
    return &banapass_plugin_descriptor;
}
