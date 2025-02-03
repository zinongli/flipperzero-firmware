#include "iso_3166.h"

static const char* lfrfid_resources_header = "Flipper LFRFID resources";
static const uint32_t lfrfid_resources_file_version = 1;

static bool
    lfrfid_search_data(Storage* storage, const char* file_name, FuriString* key, FuriString* data) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    do {
        // Open file
        if(!flipper_format_file_open_existing(file, file_name)) break;
        // Read file header and version
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, lfrfid_resources_header) ||
           (version != lfrfid_resources_file_version))
            break;
        if(!flipper_format_read_string(file, furi_string_get_cstr(key), data)) break;
        parsed = true;
    } while(false);

    furi_string_free(temp_str);
    flipper_format_free(file);
    return parsed;
}

ISO3166Country iso_3166_find_country_by_code(Storage* storage, uint16_t country_code) {
    bool parsed = false;
    FuriString* key;
    key = furi_string_alloc_printf("%04d", country_code);
    FuriString* country_names = furi_string_alloc();
    if(lfrfid_search_data(storage, EXT_PATH("lfrfid/assets/iso3166.lfrfid"), key, country_names)) {
        parsed = true;
        FURI_LOG_I(
            "iso 3166 helper",
            "Found line %d: %s",
            country_code,
            furi_string_get_cstr(country_names));
    }
    furi_string_free(key);
    if(parsed) {
        ISO3166Country country = {country_code, NULL, NULL, NULL};
        furi_string_replace_all(country_names, " ", "");
        FuriString* two_letter = furi_string_alloc_set(country_names); //AFAFGAfghanistan
        furi_string_left(two_letter, 2); // AF

        FuriString* three_letter = furi_string_alloc_set(country_names);
        furi_string_left(three_letter, 5); // AFAFG
        furi_string_right(three_letter, 2); // AFG

        FuriString* full_name = furi_string_alloc_set(country_names);
        furi_string_right(full_name, 5); // Afghanistan

        country.two_letter = furi_string_get_cstr(two_letter);
        country.three_letter = furi_string_get_cstr(three_letter);
        country.full_name = furi_string_get_cstr(full_name);

        furi_string_free(country_names);
        return country;
    }
    furi_string_free(key);
    return (ISO3166Country){0, NULL, NULL, NULL};
}
