#ifndef ORDINARY_PIPE_H
#define ORDINARY_PIPE_H

#include <windows.h>
#include <stdbool.h>

typedef struct {
    HANDLE hRead;
    HANDLE hWrite;
    bool verbose;
} OrdinaryPipe;

void ocreate_pipe(OrdinaryPipe* pipe);
void osend_message(OrdinaryPipe* pipe, const char* message);
char* oreceive_message(OrdinaryPipe* pipe);
void oclose_pipe(OrdinaryPipe* pipe);

#endif // ORDINARY_PIPE_H