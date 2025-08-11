#pragma once
#ifndef DISPENSER_PATTERN_H
#define DISPENSER_PATTERN_H

#include <stdbool.h>
#include <windows.h>

// Forward declaration
typedef struct DispenserPattern DispenserPattern;

// Dispenser modes
typedef enum {
    DISPENSER_MODE_FIFO,  // First In, First Out (queue)
    DISPENSER_MODE_LIFO,  // Last In, First Out (stack)
    DISPENSER_MODE_DEQUE  // Double-ended queue (can add/remove from both ends)
} DispenserMode;

// Item structure
typedef struct {
    unsigned char* data;
    size_t size;
} DispenserItem;

// DispenserPattern structure
typedef struct DispenserPattern {
    // Data members
    char* id;
    DispenserMode mode;
    HANDLE pipe_server;
    HANDLE pipe_client;
    DispenserItem* items;
    size_t item_count;
    size_t item_capacity;
    bool running;
    HANDLE listener_thread;
    CRITICAL_SECTION lock;
    bool verbose;

    // Method pointers
    bool (*setup)(struct DispenserPattern* self);
    bool (*add)(struct DispenserPattern* self, const unsigned char* item, size_t item_size);
    bool (*add_front)(struct DispenserPattern* self, const unsigned char* item, size_t item_size);
    bool (*add_string)(struct DispenserPattern* self, const char* item);
    bool (*add_string_front)(struct DispenserPattern* self, const char* item);
    unsigned char* (*dispense)(struct DispenserPattern* self, size_t* out_size);
    unsigned char* (*dispense_back)(struct DispenserPattern* self, size_t* out_size);
    char* (*dispense_string)(struct DispenserPattern* self);
    char* (*dispense_string_back)(struct DispenserPattern* self);
    unsigned char* (*peek)(struct DispenserPattern* self, size_t* out_size);
    unsigned char* (*peek_back)(struct DispenserPattern* self, size_t* out_size);
    char* (*peek_string)(struct DispenserPattern* self);
    char* (*peek_string_back)(struct DispenserPattern* self);
    bool (*is_empty)(struct DispenserPattern* self);
    void (*clear)(struct DispenserPattern* self);
    void (*close)(struct DispenserPattern* self);
} DispenserPattern;

// Constructor
void DispenserPattern_init(DispenserPattern* dispenser, const char* id, DispenserMode mode, bool verbose);

// Method implementations
bool DispenserPattern_setup(DispenserPattern* self);
bool DispenserPattern_add(DispenserPattern* self, const unsigned char* item, size_t item_size);
bool DispenserPattern_add_front(DispenserPattern* self, const unsigned char* item, size_t item_size);
bool DispenserPattern_add_string(DispenserPattern* self, const char* item);
bool DispenserPattern_add_string_front(DispenserPattern* self, const char* item);
unsigned char* DispenserPattern_dispense(DispenserPattern* self, size_t* out_size);
unsigned char* DispenserPattern_dispense_back(DispenserPattern* self, size_t* out_size);
char* DispenserPattern_dispense_string(DispenserPattern* self);
char* DispenserPattern_dispense_string_back(DispenserPattern* self);
unsigned char* DispenserPattern_peek(DispenserPattern* self, size_t* out_size);
unsigned char* DispenserPattern_peek_back(DispenserPattern* self, size_t* out_size);
char* DispenserPattern_peek_string(DispenserPattern* self);
char* DispenserPattern_peek_string_back(DispenserPattern* self);
bool DispenserPattern_is_empty(DispenserPattern* self);
void DispenserPattern_clear(DispenserPattern* self);
void DispenserPattern_close(DispenserPattern* self);

#endif // DISPENSER_PATTERN_H
