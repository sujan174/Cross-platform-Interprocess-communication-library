#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include <windows.h>
#include <stdbool.h>

typedef struct {
    HANDLE hPipe;
    char* pipe_name;
    bool verbose;
} NamedPipe;

void ncreate_pipe(NamedPipe* pipe);
void nconnect_pipe(NamedPipe* pipe);
void nsend_message(NamedPipe* pipe, const char* message);
char* nreceive_message(NamedPipe* pipe);
void nclose_pipe(NamedPipe* pipe);

#endif // NAMED_PIPE_H