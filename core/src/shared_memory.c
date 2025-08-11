    #include <stdio.h>
    #include "shared_memory.h"

    void setup_shared_memory(SharedMemory *shm) {
        shm->hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            shm->size,
            shm->id
        );
        if (shm->hMapFile == NULL) {
            printf("Could not create file mapping object: %lu\n", GetLastError());
        }
        shm->pBuf = MapViewOfFile(shm->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, shm->size);
    }

    void write_shared_memory(SharedMemory *shm, const char *data) {
        CopyMemory((PVOID)shm->pBuf, data, strlen(data) + 1);
    }

    char* read_shared_memory(SharedMemory *shm) {
        return (char*)shm->pBuf;
    }

    void close_shared_memory(SharedMemory *shm) {
        UnmapViewOfFile(shm->pBuf);
        CloseHandle(shm->hMapFile);
    }