#pragma once
#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include <windows.h>

typedef struct {
    const char* pipe_name;
    HANDLE hPipe;
} NamedPipe;

void ncreate_pipe(NamedPipe* pipe); // Declaration
void nconnect_pipe(NamedPipe* pipe); // Declaration
void nsend_message(NamedPipe* pipe, const char* message); // Declaration
char* nreceive_message(NamedPipe* pipe); // Declaration
void nclose_pipe(NamedPipe* pipe); // Declaration

#endif // NAMED_PIPE_H