#include "lock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void FileLock_init(FileLock* lock, const char* base_path, bool verbose) {
    
    lock->verbose = verbose;
    lock->lock_handle = INVALID_HANDLE_VALUE;
    
    
    size_t path_len = strlen(base_path);
    lock->lock_path = (char*)malloc(path_len + 6); // +6 for ".lock" and null terminator
    sprintf_s(lock->lock_path, path_len + 6, "%s.lock", base_path);
    
    // Assign method pointers
    lock->acquire = FileLock_acquire;
    lock->release = FileLock_release;
    lock->close = FileLock_close;
    
    if (lock->verbose) {
        printf("FileLock initialized with path: %s\n", lock->lock_path);
    }
}

bool FileLock_acquire(FileLock* self, DWORD timeout_ms) {
    if (self->lock_handle != INVALID_HANDLE_VALUE) {
        if (self->verbose) {
            printf("Lock already acquired\n");
        }
        return true; 
    }
    
    // Try to create the lock file with exclusive access
    self->lock_handle = CreateFileA(
        self->lock_path,           // Lock file path
        GENERIC_WRITE,             
        0,                         // No sharing - exclusive access
        NULL,                      // Default security
        CREATE_ALWAYS,             // Always create new file
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, 
        NULL);                     
    
    if (self->lock_handle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        
        // If file is already in use, wait for it to be available
        if (error == ERROR_SHARING_VIOLATION) {
            if (self->verbose) {
                printf("Lock is held by another process, waiting...\n");
            }
            
            
            DWORD start_time = GetTickCount();
            while ((GetTickCount() - start_time) < timeout_ms) {
                Sleep(10); 
                
                self->lock_handle = CreateFileA(
                    self->lock_path,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                    NULL);
                
                if (self->lock_handle != INVALID_HANDLE_VALUE) {
                    if (self->verbose) {
                        printf("Lock acquired after waiting\n");
                    }
                    return true;
                }
                
                // Only continue waiting if it's still a sharing violation
                if (GetLastError() != ERROR_SHARING_VIOLATION) {
                    break;
                }
            }
            
            if (self->verbose) {
                printf("Failed to acquire lock: timeout after %lu ms\n", timeout_ms);
            }
            return false;
        }
        else {
            if (self->verbose) {
                printf("Failed to acquire lock: error %lu\n", error);
            }
            return false;
        }
    }
    
    if (self->verbose) {
        printf("Lock acquired\n");
    }
    return true;
}

bool FileLock_release(FileLock* self) {
    if (self->lock_handle == INVALID_HANDLE_VALUE) {
        if (self->verbose) {
            printf("No lock to release\n");
        }
        return false;
    }
    
    CloseHandle(self->lock_handle);
    self->lock_handle = INVALID_HANDLE_VALUE;
    
    if (self->verbose) {
        printf("Lock released\n");
    }
    return true;
}

void FileLock_close(FileLock* self) {
    
    if (self->lock_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(self->lock_handle);
        self->lock_handle = INVALID_HANDLE_VALUE;
    }
    
    
    if (self->lock_path) {
        free(self->lock_path);
        self->lock_path = NULL;
    }
    
    if (self->verbose) {
        printf("FileLock closed\n");
    }
}
