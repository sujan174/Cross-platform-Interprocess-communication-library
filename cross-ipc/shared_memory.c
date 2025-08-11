#include <stdio.h>
#include "shared_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


void SharedMemory_init(SharedMemory* shm, const char* id, size_t size, bool verbose) {
    
    shm->id = _strdup(id);
    shm->size = size;
    shm->hFile = INVALID_HANDLE_VALUE;
    shm->hMapFile = NULL;
    shm->pBuf = NULL;
    shm->verbose = verbose;

    // Create file path in temp directory
    char temp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);

    shm->file_path = (char*)malloc(MAX_PATH);
    sprintf_s(shm->file_path, MAX_PATH, "%s\\%s.bin", temp_path, id);

    
    FileLock_init(&shm->lock, shm->file_path, verbose);

    
    shm->setup = SharedMemory_setup;
    shm->write = SharedMemory_write;
    shm->write_bytes = SharedMemory_write_bytes;
    shm->read = SharedMemory_read;
    shm->read_bytes = SharedMemory_read_bytes;
    shm->clear = SharedMemory_clear;
    shm->close = SharedMemory_close;
    shm->unlink = SharedMemory_unlink;
    shm->lock_for_writing = SharedMemory_lock_for_writing;
    shm->unlock_from_writing = SharedMemory_unlock_from_writing;

    if (shm->verbose) {
        printf("SharedMemory initialized with id '%s' and size %zu\n", id, size);
        printf("Using file: %s\n", shm->file_path);
    }
}

bool SharedMemory_setup(SharedMemory* self) {
    
    self->hFile = CreateFileA(
        self->file_path,           
        GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL,                      
        OPEN_ALWAYS,               
        FILE_ATTRIBUTE_NORMAL,     // Normal file
        NULL);                     // No template

    if (self->hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (self->verbose) {
            printf("CreateFile failed with error: %d\n", error);
        }
        return false;
    }

    
    DWORD file_size_high;
    DWORD file_size_low = GetFileSize(self->hFile, &file_size_high);

    if (file_size_low == 0 && file_size_high == 0) {
        
        DWORD high_size = 0;
        DWORD low_size = (DWORD)self->size;

        SetFilePointer(self->hFile, low_size, &high_size, FILE_BEGIN);
        SetEndOfFile(self->hFile);
        SetFilePointer(self->hFile, 0, NULL, FILE_BEGIN);

        if (self->verbose) {
            printf("Created new file with size %zu bytes\n", self->size);
        }
    }
    else if (self->verbose) {
        printf("Opened existing file with size %lu bytes\n", file_size_low);
    }

    
    self->hMapFile = CreateFileMappingA(
        self->hFile,               // File handle
        NULL,                      
        PAGE_READWRITE,            
        0,                         
        (DWORD)self->size,         
        self->id);                 

    if (self->hMapFile == NULL) {
        DWORD error = GetLastError();
        if (self->verbose) {
            printf("CreateFileMapping failed with error: %d\n", error);
        }
        CloseHandle(self->hFile);
        self->hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    
    self->pBuf = MapViewOfFile(
        self->hMapFile,            // Handle to map object
        FILE_MAP_ALL_ACCESS,       
        0,                         // High-order DWORD of offset
        0,                         
        self->size);               // Number of bytes to map

    if (self->pBuf == NULL) {
        DWORD error = GetLastError();
        if (self->verbose) {
            printf("MapViewOfFile failed with error: %d\n", error);
        }
        CloseHandle(self->hMapFile);
        self->hMapFile = NULL;
        CloseHandle(self->hFile);
        self->hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    if (self->verbose) {
        printf("SharedMemory setup complete\n");
    }

    return true;
}

bool SharedMemory_write(SharedMemory* self, const void* data, size_t data_size) {
    if (!self || !self->hMapFile || !data) {
        return false;
    }

    if (data_size > self->size) {
        if (self->verbose) {
            printf("SharedMemory_write: Data size %zu exceeds shared memory size %zu\n", 
                data_size, self->size);
        }
        return false;
    }

    
    memcpy(self->pBuf, data, data_size);

    
    FlushViewOfFile(self->pBuf, data_size);

    return true;
}

void SharedMemory_write_bytes(SharedMemory* self, const unsigned char* data, size_t data_size) {
    if (!self->pBuf) {
        if (self->verbose) {
            printf("Cannot write: shared memory not set up\n");
        }
        return;
    }

    if (data_size > self->size) {
        if (self->verbose) {
            printf("Data too large for shared memory: %zu > %zu\n", data_size, self->size);
        }
        return;
    }

    // Copy data to shared memory
    memcpy(self->pBuf, data, data_size);

    if (self->verbose) {
        printf("Wrote %zu bytes to shared memory\n", data_size);
    }

    
    FlushViewOfFile(self->pBuf, data_size);
}

char* SharedMemory_read(SharedMemory* self) {
    if (!self || !self->hMapFile) {
        return NULL;
    }

    
    char* buffer = (char*)malloc(self->size);
    if (!buffer) {
        return NULL;
    }

    
    memcpy(buffer, self->pBuf, self->size);

    return buffer;
}

unsigned char* SharedMemory_read_bytes(SharedMemory* self, size_t* out_size) {
    if (!self->pBuf) {
        if (self->verbose) {
            printf("Cannot read: shared memory not set up\n");
        }
        *out_size = 0;
        return NULL;
    }

    
    uint32_t data_size = *(uint32_t*)self->pBuf;
    
    if (data_size > self->size - sizeof(uint32_t)) {
        if (self->verbose) {
            printf("Invalid size in shared memory: %u > %zu\n", data_size, self->size - sizeof(uint32_t));
        }
        *out_size = 0;
        return NULL;
    }

    // Allocate memory for the data
    unsigned char* data = (unsigned char*)malloc(data_size);
    if (!data) {
        if (self->verbose) {
            printf("Failed to allocate memory for data\n");
        }
        *out_size = 0;
        return NULL;
    }

    
    memcpy(data, (unsigned char*)self->pBuf + sizeof(uint32_t), data_size);
    *out_size = data_size;

    if (self->verbose) {
        printf("Read %zu bytes from shared memory\n", data_size);
    }

    return data;
}

void SharedMemory_clear(SharedMemory* self) {
    if (!self->pBuf) {
        if (self->verbose) {
            printf("Cannot clear: shared memory not set up\n");
        }
        return;
    }

    // Fill with zeros
    memset(self->pBuf, 0, self->size);

    if (self->verbose) {
        printf("Shared memory cleared\n");
    }

    // Flush to ensure data is written to disk
    FlushViewOfFile(self->pBuf, self->size);
}

void SharedMemory_close(SharedMemory* self) {
    if (self->pBuf) {
        UnmapViewOfFile(self->pBuf);
        self->pBuf = NULL;

        if (self->verbose) {
            printf("Unmapped view of file\n");
        }
    }

    if (self->hMapFile) {
        CloseHandle(self->hMapFile);
        self->hMapFile = NULL;

        if (self->verbose) {
            printf("Closed file mapping handle\n");
        }
    }

    if (self->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(self->hFile);
        self->hFile = INVALID_HANDLE_VALUE;

        if (self->verbose) {
            printf("Closed file handle\n");
        }
    }
}

void SharedMemory_unlink(SharedMemory* self) {
    
    SharedMemory_close(self);

    
    self->lock.close(&self->lock);

    
    if (DeleteFileA(self->file_path)) {
        if (self->verbose) {
            printf("Deleted shared memory file: %s\n", self->file_path);
        }
    }
    else {
        DWORD error = GetLastError();
        if (self->verbose) {
            printf("Failed to delete file: %d\n", error);
        }
    }

    // Free the file path
    free(self->file_path);
    self->file_path = NULL;

    
    free(self->id);
    self->id = NULL;
}


bool SharedMemory_lock_for_writing(SharedMemory* self, DWORD timeout_ms) {
    if (self->verbose) {
        printf("Acquiring lock for writing...\n");
    }
    return self->lock.acquire(&self->lock, timeout_ms);
}

bool SharedMemory_unlock_from_writing(SharedMemory* self) {
    if (self->verbose) {
        printf("Releasing write lock...\n");
    }
    return self->lock.release(&self->lock);
}