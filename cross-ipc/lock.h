#pragma once
#ifndef LOCK_H
#define LOCK_H

#include <windows.h>
#include <stdbool.h>

typedef struct FileLock {
    // Data members
    char* lock_path;
    HANDLE lock_handle;
    bool verbose;

    // Method pointers
    bool (*acquire)(struct FileLock* self, DWORD timeout_ms);
    bool (*release)(struct FileLock* self);
    void (*close)(struct FileLock* self);
} FileLock;

// Constructor
void FileLock_init(FileLock* lock, const char* base_path, bool verbose);

// Method implementations
bool FileLock_acquire(FileLock* self, DWORD timeout_ms);
bool FileLock_release(FileLock* self);
void FileLock_close(FileLock* self);

#endif // LOCK_H
