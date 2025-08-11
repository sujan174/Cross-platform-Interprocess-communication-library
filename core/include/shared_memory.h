#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <windows.h>

typedef struct {
    const char *id;
    size_t size;
    HANDLE hMapFile;
    LPVOID pBuf;
} SharedMemory;

void setup_shared_memory(SharedMemory *shm);
void write_shared_memory(SharedMemory *shm, const char *data);
char* read_shared_memory(SharedMemory *shm);
void close_shared_memory(SharedMemory *shm);

#endif // SHARED_MEMORY_H