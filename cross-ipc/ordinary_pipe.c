#include <stdio.h>
#include "ordinary_pipe.h"


void ocreate_pipe(OrdinaryPipe* pipe) {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipe->hRead, &pipe->hWrite, &sa, 0) && pipe->verbose) {
        printf("Failed to create ordinary pipe: %lu\n", GetLastError());
    }
}

void osend_message(OrdinaryPipe* pipe, const char* message) {
    DWORD bytesWritten;
    WriteFile(pipe->hWrite, message, strlen(message) + 1, &bytesWritten, NULL);
}

char* oreceive_message(OrdinaryPipe* pipe) {
    static char buffer[65536];
    DWORD bytesRead;
    ReadFile(pipe->hRead, buffer, sizeof(buffer), &bytesRead, NULL);
    return buffer;
}

void oclose_pipe(OrdinaryPipe* pipe) {
    CloseHandle(pipe->hRead);
    CloseHandle(pipe->hWrite);
}