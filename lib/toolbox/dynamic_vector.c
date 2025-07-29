#include "dynamic_vector.h"

#define INITIAL_CAPACITY 32U

static bool dynamic_vector_reserve(DynamicVector* vector, size_t min_capacity) {
    if(min_capacity <= vector->capacity) return true;

    size_t new_capacity = vector->capacity ? vector->capacity * 2 : INITIAL_CAPACITY;
    while(new_capacity < min_capacity)
        new_capacity *= 2;

    void* new_block = realloc(vector->data, new_capacity * vector->element_size);
    if(!new_block) return false;

    vector->data = new_block;
    vector->capacity = new_capacity;
    return true;
}

bool dynamic_vector_init(DynamicVector* vector, size_t element_size) {
    furi_check(vector);
    vector->data = NULL;
    vector->count = 0;
    vector->capacity = 0;
    vector->element_size = element_size;
    return true;
}

void dynamic_vector_free(DynamicVector* vector) {
    if(!vector) return;
    free(vector->data);
    vector->data = NULL;
    vector->count = 0;
    vector->capacity = 0;
}

void dynamic_vector_clear(DynamicVector* vector) {
    if(!vector) return;
    vector->count = 0;
}

void* dynamic_vector_push_back(DynamicVector* vector) {
    furi_check(vector);
    if(!dynamic_vector_reserve(vector, vector->count + 1)) return NULL;

    void* slot = (uint8_t*)vector->data + vector->element_size * vector->count;
    memset(slot, 0, vector->element_size);
    vector->count++;
    return slot;
}

void* dynamic_vector_get(DynamicVector* vector, size_t index) {
    furi_check(vector);
    return (index < vector->count) ? (uint8_t*)vector->data + vector->element_size * index : NULL;
}

const void* dynamic_vector_cget(const DynamicVector* vector, size_t index) {
    furi_check(vector);
    return (index < vector->count) ? (const uint8_t*)vector->data + vector->element_size * index :
                                     NULL;
}
