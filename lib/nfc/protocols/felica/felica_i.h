#pragma once
#include <stdint.h>
#include <stddef.h>
#include "felica.h"

#include <lib/toolbox/simple_array.h>

extern const SimpleArrayConfig felica_service_array_cfg;
extern const SimpleArrayConfig felica_area_array_cfg;

typedef struct {
    void* data;
    size_t element_size;
    size_t count;
    size_t capacity;
} DynamicVector;

bool dynamic_vector_init(DynamicVector* vector, size_t element_size);
void dynamic_vector_free(DynamicVector* vector);
void dynamic_vector_clear(DynamicVector* vector);

void* dynamic_vector_push_back(DynamicVector* vector);
void* dynamic_vector_get(DynamicVector* vector, size_t index);
const void* dynamic_vector_cget(const DynamicVector* vector, size_t index);

static inline size_t dynamic_vector_size(const DynamicVector* vector) {
    return vector->count;
}
static inline size_t dynamic_vector_capacity(const DynamicVector* vector) {
    return vector->capacity;
}
