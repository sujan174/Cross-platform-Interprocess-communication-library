#pragma once
#ifndef SHM_DISPENSER_PATTERN_H
#define SHM_DISPENSER_PATTERN_H

#include <stdbool.h>
#include <windows.h>

// Forward declaration
typedef struct ShmDispenserPattern ShmDispenserPattern;

// Instead, include cross_ipc.h to get the enum definition
#include "cross_ipc.h"

// Shared memory structure
#pragma pack(push, 1)
typedef struct {
    int mode;                  // Dispenser mode
    size_t head;               // Index of the head (for dequeue/peek)
    size_t tail;               // Index of the tail (for enqueue)
    size_t count;              // Number of items in the dispenser
    size_t capacity;           // Maximum number of items
    size_t item_size;          // Maximum size of each item
    char data[1];              // Flexible array member for item storage
} ShmDispenserData;
#pragma pack(pop)

// ShmDispenserPattern structure
typedef struct ShmDispenserPattern {
    // Data members
    char* id;                  // Identifier for the shared memory
    ShmDispenserMode mode;     // Dispenser mode
    HANDLE shm_handle;         // Handle to shared memory
    ShmDispenserData* shm_data; // Pointer to shared memory data
    HANDLE mutex;              // Mutex for synchronization
    HANDLE not_empty_semaphore; // Semaphore to signal when dispenser is not empty
    HANDLE not_full_semaphore;  // Semaphore to signal when dispenser is not full
    bool is_provider;          // Whether this instance created the shared memory
    bool verbose;              // Whether to print verbose output

    // Method pointers
    bool (*setup)(struct ShmDispenserPattern* self, size_t capacity, size_t item_size);
    bool (*add)(struct ShmDispenserPattern* self, const void* item, size_t item_size);
    bool (*add_front)(struct ShmDispenserPattern* self, const void* item, size_t item_size);
    bool (*add_string)(struct ShmDispenserPattern* self, const char* item);
    bool (*add_string_front)(struct ShmDispenserPattern* self, const char* item);
    void* (*dispense)(struct ShmDispenserPattern* self, size_t* out_size);
    void* (*dispense_back)(struct ShmDispenserPattern* self, size_t* out_size);
    char* (*dispense_string)(struct ShmDispenserPattern* self);
    char* (*dispense_string_back)(struct ShmDispenserPattern* self);
    void* (*peek)(struct ShmDispenserPattern* self, size_t* out_size);
    void* (*peek_back)(struct ShmDispenserPattern* self, size_t* out_size);
    char* (*peek_string)(struct ShmDispenserPattern* self);
    char* (*peek_string_back)(struct ShmDispenserPattern* self);
    bool (*is_empty)(struct ShmDispenserPattern* self);
    bool (*is_full)(struct ShmDispenserPattern* self);
    void (*clear)(struct ShmDispenserPattern* self);
    void (*close)(struct ShmDispenserPattern* self);
} ShmDispenserPattern;

// Constructor
void ShmDispenserPattern_init(ShmDispenserPattern* dispenser, const char* id, ShmDispenserMode mode, bool verbose);

// Method implementations
bool ShmDispenserPattern_setup(ShmDispenserPattern* self, size_t capacity, size_t item_size);
bool ShmDispenserPattern_add(ShmDispenserPattern* self, const void* item, size_t item_size);
bool ShmDispenserPattern_add_front(ShmDispenserPattern* self, const void* item, size_t item_size);
bool ShmDispenserPattern_add_string(ShmDispenserPattern* self, const char* item);
bool ShmDispenserPattern_add_string_front(ShmDispenserPattern* self, const char* item);
void* ShmDispenserPattern_dispense(ShmDispenserPattern* self, size_t* out_size);
void* ShmDispenserPattern_dispense_back(ShmDispenserPattern* self, size_t* out_size);
char* ShmDispenserPattern_dispense_string(ShmDispenserPattern* self);
char* ShmDispenserPattern_dispense_string_back(ShmDispenserPattern* self);
void* ShmDispenserPattern_peek(ShmDispenserPattern* self, size_t* out_size);
void* ShmDispenserPattern_peek_back(ShmDispenserPattern* self, size_t* out_size);
char* ShmDispenserPattern_peek_string(ShmDispenserPattern* self);
char* ShmDispenserPattern_peek_string_back(ShmDispenserPattern* self);
bool ShmDispenserPattern_is_empty(ShmDispenserPattern* self);
bool ShmDispenserPattern_is_full(ShmDispenserPattern* self);
void ShmDispenserPattern_clear(ShmDispenserPattern* self);
void ShmDispenserPattern_close(ShmDispenserPattern* self);

#endif // SHM_DISPENSER_PATTERN_H
