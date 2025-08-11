#pragma once
#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <windows.h>
#include <stdbool.h>
#include "lock.h"

typedef struct SharedMemory {
    // Data members
    char* id;
    char* file_path;
    size_t size;
    HANDLE hFile;
    HANDLE hMapFile;
    LPVOID pBuf;
    bool verbose;
    FileLock lock;  // Added lock member

    // Method pointers
    bool (*setup)(struct SharedMemory* self);
    bool (*write)(struct SharedMemory* self, const void* data, size_t data_size);
    void (*write_bytes)(struct SharedMemory* self, const unsigned char* data, size_t data_size);
    char* (*read)(struct SharedMemory* self);
    unsigned char* (*read_bytes)(struct SharedMemory* self, size_t* out_size);
    void (*clear)(struct SharedMemory* self);
    void (*close)(struct SharedMemory* self);
    void (*unlink)(struct SharedMemory* self);
    
    // Lock-related methods
    bool (*lock_for_writing)(struct SharedMemory* self, DWORD timeout_ms);
    bool (*unlock_from_writing)(struct SharedMemory* self);
} SharedMemory;

// Constructor
void SharedMemory_init(SharedMemory* shm, const char* id, size_t size, bool verbose);

// Method implementations
bool SharedMemory_setup(SharedMemory* self);
bool SharedMemory_write(SharedMemory* self, const void* data, size_t data_size);
void SharedMemory_write_bytes(SharedMemory* self, const unsigned char* data, size_t data_size);
char* SharedMemory_read(SharedMemory* self);
unsigned char* SharedMemory_read_bytes(SharedMemory* self, size_t* out_size);
void SharedMemory_clear(SharedMemory* self);
void SharedMemory_close(SharedMemory* self);
void SharedMemory_unlink(SharedMemory* self);

// Lock-related method implementations
bool SharedMemory_lock_for_writing(SharedMemory* self, DWORD timeout_ms);
bool SharedMemory_unlock_from_writing(SharedMemory* self);

#endif // SHARED_MEMORY_H