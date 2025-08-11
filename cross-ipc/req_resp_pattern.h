#pragma once
#ifndef REQ_RESP_PATTERN_H
#define REQ_RESP_PATTERN_H

#include <stdbool.h>
#include <windows.h>

// Forward declaration
typedef struct ReqRespPattern ReqRespPattern;

// Request handler function type
typedef char* (*RequestHandler)(const char* request, void* user_data);

// ReqRespPattern structure
typedef struct ReqRespPattern {
    // Data members
    HANDLE* server_pipes;      // Array of server pipe handles
    HANDLE* client_pipes;      // Array of client pipe handles
    char** pipe_ids;           // Array of pipe IDs
    size_t pipe_count;         // Number of pipes
    size_t pipe_capacity;      // Capacity of pipe arrays
    RequestHandler* handlers;  // Array of handler functions
    void** user_data;          // Array of user data pointers
    bool running;              // Flag to control listener threads
    HANDLE* listener_threads;  // Array of listener thread handles
    CRITICAL_SECTION* locks;   // Array of critical sections for thread safety
    bool verbose;              // Verbose output flag

    // Method pointers
    bool (*setup_server)(struct ReqRespPattern* self, const char* id);
    bool (*setup_client)(struct ReqRespPattern* self, const char* id);
    char* (*request)(struct ReqRespPattern* self, const char* id, const char* message);
    void (*respond)(struct ReqRespPattern* self, const char* id, RequestHandler handler, void* user_data);
    void (*close)(struct ReqRespPattern* self);
} ReqRespPattern;

// Constructor
void ReqRespPattern_init(ReqRespPattern* rr, bool verbose);

// Method implementations
bool ReqRespPattern_setup_server(ReqRespPattern* self, const char* id);
bool ReqRespPattern_setup_client(ReqRespPattern* self, const char* id);
char* ReqRespPattern_request(ReqRespPattern* self, const char* id, const char* message);
void ReqRespPattern_respond(ReqRespPattern* self, const char* id, RequestHandler handler, void* user_data);
void ReqRespPattern_close(ReqRespPattern* self);

#endif // REQ_RESP_PATTERN_H
