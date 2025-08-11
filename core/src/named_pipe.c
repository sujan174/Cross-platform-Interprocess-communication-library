#include <stdio.h>
#include "named_pipe.h"

void ncreate_pipe(NamedPipe* pipe) {
    pipe->hPipe = CreateNamedPipe(
        pipe->pipe_name,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 65536, 65536, 0, NULL
    );
    if (pipe->hPipe == INVALID_HANDLE_VALUE) {
        printf("Failed to create named pipe: %lu\n", GetLastError());
    }
}

void nconnect_pipe(NamedPipe* pipe) {
    if (pipe->hPipe) {
        ConnectNamedPipe(pipe->hPipe, NULL);
    }
    else {
        pipe->hPipe = CreateFile(
            pipe->pipe_name,
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL
        );
        if (pipe->hPipe == INVALID_HANDLE_VALUE) {
            printf("Failed to connect to named pipe: %lu\n", GetLastError());
        }
    }
}

void nsend_message(NamedPipe* pipe, const char* message) {
    DWORD bytesWritten;
    WriteFile(pipe->hPipe, message, strlen(message) + 1, &bytesWritten, NULL);
}

char* nreceive_message(NamedPipe* pipe) {
    static char buffer[65536];
    DWORD bytesRead;
    ReadFile(pipe->hPipe, buffer, sizeof(buffer), &bytesRead, NULL);
    return buffer;
}

void nclose_pipe(NamedPipe* pipe) {
    if (pipe->hPipe) {
        CloseHandle(pipe->hPipe);
    }
}