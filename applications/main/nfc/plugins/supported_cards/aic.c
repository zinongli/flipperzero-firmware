#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>
#include <nfc/protocols/felica/felica.h>
#include <bit_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "Amusement IC"

bool aic_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);
    bool parsed = false;

    if(nfc_device_get_protocol(device) != NfcProtocolFelica) return false;

    const FelicaData* data = nfc_device_get_data(device, NfcProtocolFelica);

    const uint8_t ic_type = data->pmm.data[1];
    if(ic_type != 0xF0 && ic_type != 0xF1) {
        // Must be Felica Lite (0xF0) or Lite-S (0xF1) to parse
        return false;
    }

    const uint8_t data_format_code_1 = data->data.fs.id.data[10];
    if(data_format_code_1 != 0) {
        // We only know Data Format Code {0x00, 0xXX}
        return false;
    }

    do {
        parsed = true;
        furi_string_printf(parsed_data, "\e#Amusement IC Card\nType:\n");

        const uint8_t data_format_code_2 = data->data.fs.id.data[11];

        switch(data_format_code_2) {
        case 0x2A:
            furi_string_cat_str(parsed_data, "Bandai Namco Passport (Old)");
            break;
        case 0x3A:
            furi_string_cat_str(parsed_data, "Bandai Namco Passport (New)");
            break;
        case 0x68:
            furi_string_cat_str(parsed_data, "Konami e-Amusement Pass");
            break;
        case 0x78:
            furi_string_cat_str(parsed_data, "SEGA Aime Card");
            break;
        case 0x79:
            furi_string_cat_str(parsed_data, "Taito NESiCA");
            break;
        default:
            parsed = false;
            return parsed; // Unknown vendor
        }

    } while(false);
    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin aic_plugin = {
    .protocol = NfcProtocolFelica,
    .verify = NULL,
    .read = NULL,
    .parse = aic_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor aic_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &aic_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* aic_plugin_ep(void) {
    return &aic_plugin_descriptor;
}
