#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <storage/storage.h>
#include <flipper_format.h>
#include <furi.h>

#define ISO_3166_COUNTRY_COUNT 249

typedef struct {
    uint16_t code;            // Numeric country code (ISO 3166-1 numeric)
    const char* two_letter;  // ISO 3166-1 Alpha-2 code
    const char* three_letter;// ISO 3166-1 Alpha-3 code
    const char* full_name;   // Full country name
} ISO3166Country;

ISO3166Country iso_3166_find_country_by_code(Storage* storage, uint16_t code);
