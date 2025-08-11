#ifndef ORDINARY_PIPE_H
#define ORDINARY_PIPE_H

#include <windows.h>

typedef struct {
    HANDLE hRead;
    HANDLE hWrite;
} OrdinaryPipe;

#ifndef NAMED_PIPE_H // Prevents redefinition if named_pipe.h is included
void ocreate_pipe(OrdinaryPipe* pipe); // Declaration
void osend_message(OrdinaryPipe* pipe, const char* message); // Declaration
char* oreceive_message(OrdinaryPipe* pipe); // Declaration
void oclose_pipe(OrdinaryPipe* pipe); // Declaration
#endif // NAMED_PIPE_H

#endif // ORDINARY_PIPE_H