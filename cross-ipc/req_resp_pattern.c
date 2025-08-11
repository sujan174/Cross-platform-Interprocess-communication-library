#include "req_resp_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#define PIPE_BUFFER_SIZE 4096
#define INITIAL_CAPACITY 4


static int find_pipe_index(ReqRespPattern* self, const char* id) {
    for (size_t i = 0; i < self->pipe_count; i++) {
        if (strcmp(self->pipe_ids[i], id) == 0) {
            return (int)i;
        }
    }
    return -1;
}


static bool resize_arrays(ReqRespPattern* self, size_t new_capacity) {
    if (self->verbose) {
        printf("Resizing arrays from %zu to %zu\n", self->pipe_capacity, new_capacity);
    }

    
    HANDLE* new_server_pipes = (HANDLE*)realloc(self->server_pipes, new_capacity * sizeof(HANDLE));
    HANDLE* new_client_pipes = (HANDLE*)realloc(self->client_pipes, new_capacity * sizeof(HANDLE));
    char** new_pipe_ids = (char**)realloc(self->pipe_ids, new_capacity * sizeof(char*));
    RequestHandler* new_handlers = (RequestHandler*)realloc(self->handlers, new_capacity * sizeof(RequestHandler));
    void** new_user_data = (void**)realloc(self->user_data, new_capacity * sizeof(void*));
    HANDLE* new_listener_threads = (HANDLE*)realloc(self->listener_threads, new_capacity * sizeof(HANDLE));
    CRITICAL_SECTION* new_locks = (CRITICAL_SECTION*)realloc(self->locks, new_capacity * sizeof(CRITICAL_SECTION));

    
    if (!new_server_pipes || !new_client_pipes || !new_pipe_ids || !new_handlers || 
        !new_user_data || !new_listener_threads || !new_locks) {
        if (self->verbose) {
            printf("Failed to resize arrays\n");
        }
        return false;
    }

    
    self->server_pipes = new_server_pipes;
    self->client_pipes = new_client_pipes;
    self->pipe_ids = new_pipe_ids;
    self->handlers = new_handlers;
    self->user_data = new_user_data;
    self->listener_threads = new_listener_threads;
    self->locks = new_locks;
    self->pipe_capacity = new_capacity;

    // Initialize new elements
    for (size_t i = self->pipe_count; i < new_capacity; i++) {
        self->server_pipes[i] = INVALID_HANDLE_VALUE;
        self->client_pipes[i] = INVALID_HANDLE_VALUE;
        self->pipe_ids[i] = NULL;
        self->handlers[i] = NULL;
        self->user_data[i] = NULL;
        self->listener_threads[i] = NULL;
        InitializeCriticalSection(&self->locks[i]);
    }

    return true;
}


static unsigned __stdcall listen_for_requests(void* arg) {
    ReqRespPattern* self = ((void**)arg)[0];
    char* id = ((void**)arg)[1];
    free(arg); 

    int index = find_pipe_index(self, id);
    if (index < 0) {
        if (self->verbose) {
            printf("Error: Pipe ID '%s' not found in listener thread\n", id);
        }
        return 1;
    }

    if (self->verbose) {
        printf("Started listener thread for pipe '%s'\n", id);
    }

    while (self->running) {
        if (!self->running) {
            break;
        }

        
        if (self->verbose) {
            printf("Waiting for client connection on pipe '%s'\n", id);
        }

        BOOL result = ConnectNamedPipe(self->server_pipes[index], NULL);
        if (!result && GetLastError() != ERROR_PIPE_CONNECTED) {
            if (self->verbose) {
                printf("Error connecting pipe '%s': %d\n", id, GetLastError());
            }
            Sleep(1000); // Avoid tight loop on error
            continue;
        }

        if (self->verbose) {
            printf("Client connected to pipe '%s'\n", id);
        }

        while (self->running) {
            
            char buffer[PIPE_BUFFER_SIZE];
            DWORD bytes_read = 0;
            BOOL read_result = ReadFile(
                self->server_pipes[index],
                buffer,
                PIPE_BUFFER_SIZE - 1,
                &bytes_read,
                NULL
            );

            if (!read_result || bytes_read == 0) {
                if (self->verbose) {
                    printf("Client disconnected from pipe '%s'\n", id);
                }
                break;
            }

            
            buffer[bytes_read] = '\0';

            if (self->verbose) {
                printf("Received request on pipe '%s': %s\n", id, buffer);
            }

            // Process request through handler
            char* response = NULL;
            if (self->handlers[index]) {
                response = self->handlers[index](buffer, self->user_data[index]);
            } else {
                response = _strdup("Error: No handler registered");
            }

            if (!response) {
                response = _strdup("Error: Handler returned NULL");
            }

            
            DWORD bytes_written = 0;
            BOOL write_result = WriteFile(
                self->server_pipes[index],
                response,
                (DWORD)strlen(response),
                &bytes_written,
                NULL
            );

            if (!write_result || bytes_written != strlen(response)) {
                if (self->verbose) {
                    printf("Error sending response on pipe '%s': %d\n", id, GetLastError());
                }
            }

            
            free(response);
        }

        // Disconnect the client
        DisconnectNamedPipe(self->server_pipes[index]);
    }

    if (self->verbose) {
        printf("Listener thread for pipe '%s' exiting\n", id);
    }

    return 0;
}


void ReqRespPattern_init(ReqRespPattern* rr, bool verbose) {
    // Initialize data members
    rr->server_pipes = NULL;
    rr->client_pipes = NULL;
    rr->pipe_ids = NULL;
    rr->pipe_count = 0;
    rr->pipe_capacity = 0;
    rr->handlers = NULL;
    rr->user_data = NULL;
    rr->running = false;
    rr->listener_threads = NULL;
    rr->locks = NULL;
    rr->verbose = verbose;

    // Initialize method pointers
    rr->setup_server = ReqRespPattern_setup_server;
    rr->setup_client = ReqRespPattern_setup_client;
    rr->request = ReqRespPattern_request;
    rr->respond = ReqRespPattern_respond;
    rr->close = ReqRespPattern_close;

    // Allocate initial arrays
    resize_arrays(rr, INITIAL_CAPACITY);

    if (verbose) {
        printf("ReqRespPattern initialized\n");
    }
}

bool ReqRespPattern_setup_server(ReqRespPattern* self, const char* id) {
    if (self->verbose) {
        printf("Setting up server for pipe '%s'\n", id);
    }

    
    int index = find_pipe_index(self, id);
    if (index >= 0) {
        if (self->verbose) {
            printf("Pipe '%s' already exists\n", id);
        }
        return false;
    }

    
    if (self->pipe_count >= self->pipe_capacity) {
        if (!resize_arrays(self, self->pipe_capacity * 2)) {
            return false;
        }
    }

    
    char pipe_name[256];
    sprintf_s(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\reqresp_%s", id);

    // Create the named pipe
    HANDLE pipe = CreateNamedPipeA(
        pipe_name,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        0,
        NULL
    );

    if (pipe == INVALID_HANDLE_VALUE) {
        if (self->verbose) {
            printf("Failed to create named pipe '%s'\n", pipe_name);
        }
        return false;
    }

    
    self->server_pipes[self->pipe_count] = pipe;
    self->client_pipes[self->pipe_count] = INVALID_HANDLE_VALUE;
    self->pipe_ids[self->pipe_count] = _strdup(id);
    self->handlers[self->pipe_count] = NULL;
    self->user_data[self->pipe_count] = NULL;
    self->listener_threads[self->pipe_count] = NULL;

    
    self->running = true;
    
    
    void** thread_args = (void**)malloc(2 * sizeof(void*));
    thread_args[0] = self;
    thread_args[1] = self->pipe_ids[self->pipe_count];
    
    self->listener_threads[self->pipe_count] = (HANDLE)_beginthreadex(
        NULL, 0, listen_for_requests, thread_args, 0, NULL
    );

    if (!self->listener_threads[self->pipe_count]) {
        if (self->verbose) {
            printf("Failed to create listener thread for pipe '%s'\n", id);
        }
        CloseHandle(pipe);
        free(self->pipe_ids[self->pipe_count]);
        return false;
    }

    self->pipe_count++;

    if (self->verbose) {
        printf("Server set up for pipe '%s'\n", id);
    }

    return true;
}

bool ReqRespPattern_setup_client(ReqRespPattern* self, const char* id) {
    if (self->verbose) {
        printf("Setting up client for pipe '%s'\n", id);
    }

    
    int index = find_pipe_index(self, id);
    if (index >= 0) {
        if (self->verbose) {
            printf("Pipe '%s' already exists\n", id);
        }
        return false;
    }

    
    if (self->pipe_count >= self->pipe_capacity) {
        if (!resize_arrays(self, self->pipe_capacity * 2)) {
            return false;
        }
    }

    
    char pipe_name[256];
    sprintf_s(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\reqresp_%s", id);

    // Connect to the named pipe
    HANDLE pipe = CreateFileA(
        pipe_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (pipe == INVALID_HANDLE_VALUE) {
        if (self->verbose) {
            printf("Failed to connect to named pipe '%s'\n", pipe_name);
        }
        return false;
    }

    // Set the pipe mode
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe, &mode, NULL, NULL)) {
        if (self->verbose) {
            printf("Failed to set pipe mode for '%s'\n", pipe_name);
        }
        CloseHandle(pipe);
        return false;
    }

    // Add the pipe to our arrays
    self->server_pipes[self->pipe_count] = INVALID_HANDLE_VALUE;
    self->client_pipes[self->pipe_count] = pipe;
    self->pipe_ids[self->pipe_count] = _strdup(id);
    self->handlers[self->pipe_count] = NULL;
    self->user_data[self->pipe_count] = NULL;
    self->listener_threads[self->pipe_count] = NULL;

    self->pipe_count++;

    if (self->verbose) {
        printf("Client set up for pipe '%s'\n", id);
    }

    return true;
}

char* ReqRespPattern_request(ReqRespPattern* self, const char* id, const char* message) {
    int index = find_pipe_index(self, id);
    if (index < 0) {
        if (self->verbose) {
            printf("Error: Pipe ID '%s' not found for request\n", id);
        }
        return NULL;
    }

    // Acquire the lock
    EnterCriticalSection(&self->locks[index]);

    char* response = NULL;

    
    DWORD bytes_written = 0;
    BOOL write_result = WriteFile(
        self->client_pipes[index],
        message,
        (DWORD)strlen(message),
        &bytes_written,
        NULL
    );

    if (!write_result || bytes_written != strlen(message)) {
        if (self->verbose) {
            printf("Error sending request on pipe '%s': %d\n", id, GetLastError());
        }
        LeaveCriticalSection(&self->locks[index]);
        return NULL;
    }

    
    char buffer[PIPE_BUFFER_SIZE];
    DWORD bytes_read = 0;
    BOOL read_result = ReadFile(
        self->client_pipes[index],
        buffer,
        PIPE_BUFFER_SIZE - 1,
        &bytes_read,
        NULL
    );

    if (!read_result || bytes_read == 0) {
        if (self->verbose) {
            printf("Error reading response from pipe '%s': %d\n", id, GetLastError());
        }
        LeaveCriticalSection(&self->locks[index]);
        return NULL;
    }

    
    buffer[bytes_read] = '\0';

    if (self->verbose) {
        printf("Received response from pipe '%s': %s\n", id, buffer);
    }

    // Return a copy of the response
    response = _strdup(buffer);

    // Release the lock
    LeaveCriticalSection(&self->locks[index]);

    return response;
}

void ReqRespPattern_respond(ReqRespPattern* self, const char* id, RequestHandler handler, void* user_data) {
    int index = find_pipe_index(self, id);
    if (index < 0) {
        if (self->verbose) {
            printf("Error: Pipe ID '%s' not found for respond\n", id);
        }
        return;
    }

    self->handlers[index] = handler;
    self->user_data[index] = user_data;

    if (self->verbose) {
        printf("Set handler for pipe '%s'\n", id);
    }
}

void ReqRespPattern_close(ReqRespPattern* self) {
    if (self->verbose) {
        printf("Closing ReqRespPattern\n");
    }

    // Stop all listener threads
    self->running = false;

    // Wait for listener threads to finish
    for (size_t i = 0; i < self->pipe_count; i++) {
        if (self->listener_threads[i] && self->listener_threads[i] != INVALID_HANDLE_VALUE) {
            if (self->verbose) {
                printf("Waiting for listener thread for pipe '%s' to finish\n", self->pipe_ids[i]);
            }
            WaitForSingleObject(self->listener_threads[i], 1000);
            CloseHandle(self->listener_threads[i]);
            self->listener_threads[i] = NULL;
        }
    }

    
    for (size_t i = 0; i < self->pipe_count; i++) {
        if (self->server_pipes[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(self->server_pipes[i]);
            self->server_pipes[i] = INVALID_HANDLE_VALUE;
        }
        if (self->client_pipes[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(self->client_pipes[i]);
            self->client_pipes[i] = INVALID_HANDLE_VALUE;
        }
    }

    
    for (size_t i = 0; i < self->pipe_count; i++) {
        free(self->pipe_ids[i]);
        DeleteCriticalSection(&self->locks[i]);
    }

    free(self->server_pipes);
    free(self->client_pipes);
    free(self->pipe_ids);
    free(self->handlers);
    free(self->user_data);
    free(self->listener_threads);
    free(self->locks);

    self->server_pipes = NULL;
    self->client_pipes = NULL;
    self->pipe_ids = NULL;
    self->handlers = NULL;
    self->user_data = NULL;
    self->listener_threads = NULL;
    self->locks = NULL;
    self->pipe_count = 0;
    self->pipe_capacity = 0;

    if (self->verbose) {
        printf("ReqRespPattern closed\n");
    }
}
