#include "felica_i.h"

const SimpleArrayConfig felica_service_array_cfg = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(FelicaService),
};

const SimpleArrayConfig felica_area_array_cfg = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(FelicaArea),
};
