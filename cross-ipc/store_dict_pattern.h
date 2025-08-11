#pragma once
#ifndef STORE_DICT_PATTERN_H
#define STORE_DICT_PATTERN_H

#include <stdbool.h>
#include "shared_memory.h"
#include <windows.h>
#include <stdint.h>  // Add this for uint32_t

// Forward declaration
typedef struct StoreDictPattern StoreDictPattern;

// Entry structure
typedef struct {
    char* key;
    unsigned char* value;
    size_t value_size;
} DictEntry;

// StoreDictPattern structure
typedef struct StoreDictPattern {
    // Data members
    char* id;
    size_t size;
    SharedMemory shm;
    DictEntry* entries;
    size_t entry_count;
    size_t entry_capacity;
    bool verbose;
    HANDLE mutex;  // Named mutex for cross-process synchronization
    uint32_t version;  // Version number for change tracking

    // Method pointers
    bool (*setup)(struct StoreDictPattern* self);
    bool (*store)(struct StoreDictPattern* self, const char* key, const unsigned char* value, size_t value_size);
    void (*store_string)(struct StoreDictPattern* self, const char* key, const char* value);
    void (*store_bytes)(struct StoreDictPattern* self, const char* key, const unsigned char* value, size_t value_size);
    unsigned char* (*retrieve)(struct StoreDictPattern* self, const char* key, size_t* out_size);
    unsigned char* (*retrieve_bytes)(struct StoreDictPattern* self, const char* key, size_t* out_size);
    char* (*retrieve_string)(struct StoreDictPattern* self, const char* key);
    void (*load)(struct StoreDictPattern* self);
    bool (*sync)(struct StoreDictPattern* self);
    char** (*list_keys)(struct StoreDictPattern* self, size_t* out_count);
    void (*close)(struct StoreDictPattern* self);
} StoreDictPattern;

// Constructor
void StoreDictPattern_init(StoreDictPattern* store, const char* id, size_t size, bool verbose);

// Method implementations
bool StoreDictPattern_setup(StoreDictPattern* self);
bool StoreDictPattern_store(StoreDictPattern* self, const char* key, const unsigned char* value, size_t value_size);
void StoreDictPattern_store_string(StoreDictPattern* self, const char* key, const char* value);
void StoreDictPattern_store_bytes(StoreDictPattern* self, const char* key, const unsigned char* value, size_t value_size);
unsigned char* StoreDictPattern_retrieve(StoreDictPattern* self, const char* key, size_t* out_size);
unsigned char* StoreDictPattern_retrieve_bytes(StoreDictPattern* self, const char* key, size_t* out_size);
char* StoreDictPattern_retrieve_string(StoreDictPattern* self, const char* key);
void StoreDictPattern_load(StoreDictPattern* self);
bool StoreDictPattern_sync(StoreDictPattern* self);
char** StoreDictPattern_list_keys(StoreDictPattern* self, size_t* out_count);
void StoreDictPattern_close(StoreDictPattern* self);

#endif // STORE_DICT_PATTERN_H