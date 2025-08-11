#include "dispenser_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#define PIPE_BUFFER_SIZE 4096
#define INITIAL_CAPACITY 16


static bool resize_items(DispenserPattern* self, size_t new_capacity) {
    if (self->verbose) {
        printf("Resizing items from %zu to %zu\n", self->item_capacity, new_capacity);
    }

    DispenserItem* new_items = (DispenserItem*)realloc(self->items, new_capacity * sizeof(DispenserItem));
    if (!new_items) {
        if (self->verbose) {
            printf("Failed to resize items array\n");
        }
        return false;
    }

    self->items = new_items;
    self->item_capacity = new_capacity;
    return true;
}

// Thread function for listening to remote operations
static unsigned __stdcall listener_thread_func(void* arg) {
    DispenserPattern* self = (DispenserPattern*)arg;
    
    if (self->verbose) {
        printf("Started listener thread for dispenser '%s'\n", self->id);
    }
    
    while (self->running) {
        
        if (self->verbose) {
            printf("Waiting for client connection on dispenser '%s'\n", self->id);
        }
        
        BOOL result = ConnectNamedPipe(self->pipe_server, NULL);
        if (!result && GetLastError() != ERROR_PIPE_CONNECTED) {
            if (self->verbose) {
                printf("Error connecting pipe for dispenser '%s': %d\n", self->id, GetLastError());
            }
            Sleep(1000); 
            continue;
        }
        
        if (self->verbose) {
            printf("Client connected to dispenser '%s'\n", self->id);
        }
        
        while (self->running) {
            
            char buffer[PIPE_BUFFER_SIZE];
            DWORD bytes_read = 0;
            BOOL read_result = ReadFile(
                self->pipe_server,
                buffer,
                PIPE_BUFFER_SIZE - 1,
                &bytes_read,
                NULL
            );
            
            if (!read_result || bytes_read == 0) {
                if (self->verbose) {
                    printf("Client disconnected from dispenser '%s'\n", self->id);
                }
                break;
            }
            
            
            buffer[bytes_read] = '\0';
            
            if (self->verbose) {
                printf("Received command on dispenser '%s': %s\n", self->id, buffer);
            }
            
            
            char response[PIPE_BUFFER_SIZE] = {0};
            
            
            EnterCriticalSection(&self->lock);
            
            
            char cmd_upper[PIPE_BUFFER_SIZE];
            strcpy_s(cmd_upper, PIPE_BUFFER_SIZE, buffer);
            for (int i = 0; cmd_upper[i]; i++) {
                cmd_upper[i] = toupper(cmd_upper[i]);
            }
            
            if (strncmp(cmd_upper, "ADD ", 4) == 0) {
                
                const char* item = buffer + 4;
                if (self->add_string(self, item)) {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "OK");
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "ERROR: Failed to add item");
                }
            }
            else if (strncmp(cmd_upper, "ADD_FRONT ", 10) == 0) {
                // Add item to the front
                const char* item = buffer + 10;
                if (self->add_string_front(self, item)) {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "OK");
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "ERROR: Failed to add item to front");
                }
            }
            else if (strcmp(cmd_upper, "DISPENSE") == 0) {
                
                char* item = self->dispense_string(self);
                if (item) {
                    snprintf(response, PIPE_BUFFER_SIZE, "ITEM: %s", item);
                    free(item);
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "ERROR: Dispenser is empty");
                }
            }
            else if (strcmp(cmd_upper, "DISPENSE_BACK") == 0) {
                // Dispense item from the back
                char* item = self->dispense_string_back(self);
                if (item) {
                    snprintf(response, PIPE_BUFFER_SIZE, "ITEM: %s", item);
                    free(item);
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "ERROR: Dispenser is empty");
                }
            }
            else if (strcmp(cmd_upper, "PEEK") == 0) {
                // Peek at the front item
                char* item = self->peek_string(self);
                if (item) {
                    snprintf(response, PIPE_BUFFER_SIZE, "ITEM: %s", item);
                    free(item);
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "ERROR: Dispenser is empty");
                }
            }
            else if (strcmp(cmd_upper, "PEEK_BACK") == 0) {
                
                char* item = self->peek_string_back(self);
                if (item) {
                    snprintf(response, PIPE_BUFFER_SIZE, "ITEM: %s", item);
                    free(item);
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "ERROR: Dispenser is empty or operation not supported");
                }
            }
            else if (strcmp(cmd_upper, "IS_EMPTY") == 0) {
                
                if (self->is_empty(self)) {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "TRUE");
                } else {
                    strcpy_s(response, PIPE_BUFFER_SIZE, "FALSE");
                }
            }
            else if (strcmp(cmd_upper, "CLEAR") == 0) {
                
                self->clear(self);
                strcpy_s(response, PIPE_BUFFER_SIZE, "OK");
            }
            else {
                // Unknown command
                snprintf(response, PIPE_BUFFER_SIZE, "ERROR: Unknown command '%s'", buffer);
            }
            
            LeaveCriticalSection(&self->lock);
            
            // Send response
            DWORD bytes_written = 0;
            BOOL write_result = WriteFile(
                self->pipe_server,
                response,
                (DWORD)strlen(response),
                &bytes_written,
                NULL
            );
            
            if (!write_result || bytes_written != strlen(response)) {
                if (self->verbose) {
                    printf("Error sending response on dispenser '%s': %d\n", self->id, GetLastError());
                }
                break;
            }
        }
        
        
        DisconnectNamedPipe(self->pipe_server);
    }
    
    if (self->verbose) {
        printf("Listener thread for dispenser '%s' exiting\n", self->id);
    }
    
    return 0;
}

// Constructor implementation
void DispenserPattern_init(DispenserPattern* dispenser, const char* id, DispenserMode mode, bool verbose) {
    
    dispenser->id = _strdup(id);
    dispenser->mode = mode;
    dispenser->pipe_server = INVALID_HANDLE_VALUE;
    dispenser->pipe_client = INVALID_HANDLE_VALUE;
    dispenser->items = NULL;
    dispenser->item_count = 0;
    dispenser->item_capacity = 0;
    dispenser->running = false;
    dispenser->listener_thread = NULL;
    InitializeCriticalSection(&dispenser->lock);
    dispenser->verbose = verbose;
    
    // Initialize method pointers
    dispenser->setup = DispenserPattern_setup;
    dispenser->add = DispenserPattern_add;
    dispenser->add_front = DispenserPattern_add_front;
    dispenser->add_string = DispenserPattern_add_string;
    dispenser->add_string_front = DispenserPattern_add_string_front;
    dispenser->dispense = DispenserPattern_dispense;
    dispenser->dispense_back = DispenserPattern_dispense_back;
    dispenser->dispense_string = DispenserPattern_dispense_string;
    dispenser->dispense_string_back = DispenserPattern_dispense_string_back;
    dispenser->peek = DispenserPattern_peek;
    dispenser->peek_back = DispenserPattern_peek_back;
    dispenser->peek_string = DispenserPattern_peek_string;
    dispenser->peek_string_back = DispenserPattern_peek_string_back;
    dispenser->is_empty = DispenserPattern_is_empty;
    dispenser->clear = DispenserPattern_clear;
    dispenser->close = DispenserPattern_close;
    
    
    resize_items(dispenser, INITIAL_CAPACITY);
    
    if (verbose) {
        printf("DispenserPattern initialized with id '%s' and mode %d\n", id, mode);
    }
}


bool DispenserPattern_setup(DispenserPattern* self) {
    if (self->verbose) {
        printf("Setting up dispenser '%s'\n", self->id);
    }
    
    
    char pipe_name[256];
    sprintf_s(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\dispenser_%s", self->id);
    
    
    self->pipe_server = CreateNamedPipeA(
        pipe_name,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        0,
        NULL
    );
    
    if (self->pipe_server == INVALID_HANDLE_VALUE) {
        if (self->verbose) {
            printf("Failed to create server pipe for dispenser '%s': %d\n", self->id, GetLastError());
        }
        return false;
    }
    
    // Start listener thread for server pipe
    self->running = true;
    self->listener_thread = (HANDLE)_beginthreadex(NULL, 0, listener_thread_func, self, 0, NULL);
    
    if (!self->listener_thread) {
        if (self->verbose) {
            printf("Failed to create listener thread for dispenser '%s'\n", self->id);
        }
        CloseHandle(self->pipe_server);
        self->pipe_server = INVALID_HANDLE_VALUE;
        return false;
    }
    
    if (self->verbose) {
        printf("Dispenser '%s' set up successfully\n", self->id);
    }
    
    return true;
}

bool DispenserPattern_add(DispenserPattern* self, const unsigned char* item, size_t item_size) {
    if (!item || item_size == 0) {
        return false;
    }
    
    EnterCriticalSection(&self->lock);
    
    
    if (self->item_count >= self->item_capacity) {
        if (!resize_items(self, self->item_capacity * 2)) {
            LeaveCriticalSection(&self->lock);
            return false;
        }
    }
    
    // Allocate memory for the item
    unsigned char* item_copy = (unsigned char*)malloc(item_size);
    if (!item_copy) {
        LeaveCriticalSection(&self->lock);
        return false;
    }
    
    
    memcpy(item_copy, item, item_size);
    
    
    self->items[self->item_count].data = item_copy;
    self->items[self->item_count].size = item_size;
    self->item_count++;
    
    if (self->verbose) {
        printf("Added item to dispenser '%s', new count: %zu\n", self->id, self->item_count);
    }
    
    LeaveCriticalSection(&self->lock);
    return true;
}

bool DispenserPattern_add_front(DispenserPattern* self, const unsigned char* item, size_t item_size) {
    if (!item || item_size == 0) {
        return false;
    }
    
    EnterCriticalSection(&self->lock);
    
    // Resize if needed
    if (self->item_count >= self->item_capacity) {
        if (!resize_items(self, self->item_capacity * 2)) {
            LeaveCriticalSection(&self->lock);
            return false;
        }
    }
    
    // Allocate memory for the item
    unsigned char* item_copy = (unsigned char*)malloc(item_size);
    if (!item_copy) {
        LeaveCriticalSection(&self->lock);
        return false;
    }
    
    // Copy the item
    memcpy(item_copy, item, item_size);
    
    
    for (size_t i = self->item_count; i > 0; i--) {
        self->items[i] = self->items[i - 1];
    }
    
    
    self->items[0].data = item_copy;
    self->items[0].size = item_size;
    self->item_count++;
    
    if (self->verbose) {
        printf("Added item to front of dispenser '%s', new count: %zu\n", self->id, self->item_count);
    }
    
    LeaveCriticalSection(&self->lock);
    return true;
}

bool DispenserPattern_add_string(DispenserPattern* self, const char* item) {
    return self->add(self, (const unsigned char*)item, strlen(item) + 1);
}

bool DispenserPattern_add_string_front(DispenserPattern* self, const char* item) {
    return self->add_front(self, (const unsigned char*)item, strlen(item) + 1);
}

unsigned char* DispenserPattern_dispense(DispenserPattern* self, size_t* out_size) {
    EnterCriticalSection(&self->lock);
    
    if (self->item_count == 0) {
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    unsigned char* item_data = NULL;
    size_t item_size = 0;
    
    if (self->mode == DISPENSER_MODE_FIFO) {
        
        item_data = self->items[0].data;
        item_size = self->items[0].size;
        
        // Shift remaining items
        for (size_t i = 0; i < self->item_count - 1; i++) {
            self->items[i] = self->items[i + 1];
        }
    }
    else {
        // LIFO or DEQUE: remove from back
        item_data = self->items[self->item_count - 1].data;
        item_size = self->items[self->item_count - 1].size;
    }
    
    
    self->item_count--;
    
    if (self->verbose) {
        printf("Dispensed item from dispenser '%s', new count: %zu\n", self->id, self->item_count);
    }
    
    if (out_size) {
        *out_size = item_size;
    }
    
    LeaveCriticalSection(&self->lock);
    return item_data;
}

unsigned char* DispenserPattern_dispense_back(DispenserPattern* self, size_t* out_size) {
    EnterCriticalSection(&self->lock);
    
    if (self->item_count == 0) {
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    unsigned char* item_data = NULL;
    size_t item_size = 0;
    
    if (self->mode == DISPENSER_MODE_DEQUE) {
        // DEQUE: can remove from back
        item_data = self->items[self->item_count - 1].data;
        item_size = self->items[self->item_count - 1].size;
    }
    else {
        
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    // Decrement count
    self->item_count--;
    
    if (self->verbose) {
        printf("Dispensed item from back of dispenser '%s', new count: %zu\n", self->id, self->item_count);
    }
    
    if (out_size) {
        *out_size = item_size;
    }
    
    LeaveCriticalSection(&self->lock);
    return item_data;
}

char* DispenserPattern_dispense_string(DispenserPattern* self) {
    size_t size;
    return (char*)self->dispense(self, &size);
}

char* DispenserPattern_dispense_string_back(DispenserPattern* self) {
    size_t size;
    return (char*)self->dispense_back(self, &size);
}

unsigned char* DispenserPattern_peek(DispenserPattern* self, size_t* out_size) {
    EnterCriticalSection(&self->lock);
    
    if (self->item_count == 0) {
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    unsigned char* item_data = NULL;
    size_t item_size = 0;
    
    if (self->mode == DISPENSER_MODE_FIFO) {
        // FIFO: peek at front
        item_data = self->items[0].data;
        item_size = self->items[0].size;
    }
    else {
        
        item_data = self->items[self->item_count - 1].data;
        item_size = self->items[self->item_count - 1].size;
    }
    
    
    unsigned char* data_copy = (unsigned char*)malloc(item_size);
    if (!data_copy) {
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    memcpy(data_copy, item_data, item_size);
    
    if (out_size) {
        *out_size = item_size;
    }
    
    LeaveCriticalSection(&self->lock);
    return data_copy;
}

unsigned char* DispenserPattern_peek_back(DispenserPattern* self, size_t* out_size) {
    EnterCriticalSection(&self->lock);
    
    if (self->item_count == 0) {
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    unsigned char* item_data = NULL;
    size_t item_size = 0;
    
    if (self->mode == DISPENSER_MODE_DEQUE) {
        
        item_data = self->items[self->item_count - 1].data;
        item_size = self->items[self->item_count - 1].size;
    }
    else {
        
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    // Make a copy of the data
    unsigned char* data_copy = (unsigned char*)malloc(item_size);
    if (!data_copy) {
        LeaveCriticalSection(&self->lock);
        return NULL;
    }
    
    memcpy(data_copy, item_data, item_size);
    
    if (out_size) {
        *out_size = item_size;
    }
    
    LeaveCriticalSection(&self->lock);
    return data_copy;
}

char* DispenserPattern_peek_string(DispenserPattern* self) {
    size_t size;
    return (char*)self->peek(self, &size);
}

char* DispenserPattern_peek_string_back(DispenserPattern* self) {
    size_t size;
    return (char*)self->peek_back(self, &size);
}

bool DispenserPattern_is_empty(DispenserPattern* self) {
    EnterCriticalSection(&self->lock);
    bool empty = (self->item_count == 0);
    LeaveCriticalSection(&self->lock);
    return empty;
}

void DispenserPattern_clear(DispenserPattern* self) {
    EnterCriticalSection(&self->lock);
    
    
    for (size_t i = 0; i < self->item_count; i++) {
        free(self->items[i].data);
    }
    
    self->item_count = 0;
    
    if (self->verbose) {
        printf("Cleared dispenser '%s'\n", self->id);
    }
    
    LeaveCriticalSection(&self->lock);
}

void DispenserPattern_close(DispenserPattern* self) {
    if (self->verbose) {
        printf("Closing dispenser '%s'\n", self->id);
    }
    
    // Stop the listener thread
    self->running = false;
    
    
    if (self->listener_thread) {
        if (self->verbose) {
            printf("Waiting for listener thread to finish\n");
        }
        WaitForSingleObject(self->listener_thread, 1000);
        CloseHandle(self->listener_thread);
        self->listener_thread = NULL;
    }
    
    
    if (self->pipe_server != INVALID_HANDLE_VALUE) {
        CloseHandle(self->pipe_server);
        self->pipe_server = INVALID_HANDLE_VALUE;
    }
    
    if (self->pipe_client != INVALID_HANDLE_VALUE) {
        CloseHandle(self->pipe_client);
        self->pipe_client = INVALID_HANDLE_VALUE;
    }
    
    // Clear items
    self->clear(self);
    
    // Free items array
    free(self->items);
    self->items = NULL;
    self->item_capacity = 0;
    
    
    free(self->id);
    self->id = NULL;
    
    
    DeleteCriticalSection(&self->lock);
    
    if (self->verbose) {
        printf("Dispenser closed\n");
    }
}
