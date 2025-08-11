 #include "shm_dispenser_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_ITEM_SIZE 256
#define DEFAULT_CAPACITY 16


static size_t calculate_shm_size(size_t capacity, size_t item_size) {
    // Size of header + size of all items
    
    return sizeof(ShmDispenserData) - 1 + (capacity * (sizeof(size_t) + item_size));
}

// Helper function to get pointer to item at specific index
static char* get_item_ptr(ShmDispenserData* data, size_t index) {
    // Calculate offset: skip header and all previous items
    size_t offset = index * (sizeof(size_t) + data->item_size);
    return &data->data[offset];
}


static size_t get_item_size(ShmDispenserData* data, size_t index) {
    char* ptr = get_item_ptr(data, index);
    return *((size_t*)ptr);
}

// Helper function to get data of item at specific index
static char* get_item_data(ShmDispenserData* data, size_t index) {
    char* ptr = get_item_ptr(data, index);
    return ptr + sizeof(size_t);
}

// Helper function to set item at specific index
static void set_item(ShmDispenserData* data, size_t index, const void* item, size_t item_size) {
    char* ptr = get_item_ptr(data, index);
    *((size_t*)ptr) = item_size;
    memcpy(ptr + sizeof(size_t), item, item_size);
}


void ShmDispenserPattern_init(ShmDispenserPattern* dispenser, const char* id, ShmDispenserMode mode, bool verbose) {
    dispenser->id = _strdup(id);
    dispenser->mode = mode;
    dispenser->shm_handle = NULL;
    dispenser->shm_data = NULL;
    dispenser->mutex = NULL;
    dispenser->not_empty_semaphore = NULL;
    dispenser->not_full_semaphore = NULL;
    dispenser->is_provider = false;
    dispenser->verbose = verbose;

    
    dispenser->setup = ShmDispenserPattern_setup;
    dispenser->add = ShmDispenserPattern_add;
    dispenser->add_front = ShmDispenserPattern_add_front;
    dispenser->add_string = ShmDispenserPattern_add_string;
    dispenser->add_string_front = ShmDispenserPattern_add_string_front;
    dispenser->dispense = ShmDispenserPattern_dispense;
    dispenser->dispense_back = ShmDispenserPattern_dispense_back;
    dispenser->dispense_string = ShmDispenserPattern_dispense_string;
    dispenser->dispense_string_back = ShmDispenserPattern_dispense_string_back;
    dispenser->peek = ShmDispenserPattern_peek;
    dispenser->peek_back = ShmDispenserPattern_peek_back;
    dispenser->peek_string = ShmDispenserPattern_peek_string;
    dispenser->peek_string_back = ShmDispenserPattern_peek_string_back;
    dispenser->is_empty = ShmDispenserPattern_is_empty;
    dispenser->is_full = ShmDispenserPattern_is_full;
    dispenser->clear = ShmDispenserPattern_clear;
    dispenser->close = ShmDispenserPattern_close;

    if (verbose) {
        printf("ShmDispenserPattern initialized with id '%s' and mode %d\n", id, mode);
    }
}

// Setup method
bool ShmDispenserPattern_setup(ShmDispenserPattern* self, size_t capacity, size_t item_size) {
    if (self->verbose) {
        printf("Setting up shared memory dispenser '%s'\n", self->id);
    }

    if (capacity == 0) capacity = DEFAULT_CAPACITY;
    if (item_size == 0) item_size = DEFAULT_ITEM_SIZE;

    
    char mutex_name[256];
    sprintf_s(mutex_name, sizeof(mutex_name), "ShmDispenser_Mutex_%s", self->id);

    
    char not_empty_name[256];
    sprintf_s(not_empty_name, sizeof(not_empty_name), "ShmDispenser_NotEmpty_%s", self->id);

    char not_full_name[256];
    sprintf_s(not_full_name, sizeof(not_full_name), "ShmDispenser_NotFull_%s", self->id);

    
    char shm_name[256];
    sprintf_s(shm_name, sizeof(shm_name), "ShmDispenser_%s", self->id);

    
    size_t shm_size = calculate_shm_size(capacity, item_size);

    
    self->shm_handle = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        shm_name
    );

    if (self->shm_handle == NULL) {
        
        self->shm_handle = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            (DWORD)shm_size,
            shm_name
        );

        if (self->shm_handle == NULL) {
            if (self->verbose) {
                printf("Failed to create shared memory for dispenser '%s': %d\n", self->id, GetLastError());
            }
            return false;
        }

        self->is_provider = true;
    }

    // Map shared memory
    self->shm_data = (ShmDispenserData*)MapViewOfFile(
        self->shm_handle,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        shm_size
    );

    if (self->shm_data == NULL) {
        if (self->verbose) {
            printf("Failed to map shared memory for dispenser '%s': %d\n", self->id, GetLastError());
        }
        CloseHandle(self->shm_handle);
        self->shm_handle = NULL;
        return false;
    }

    
    if (self->is_provider) {
        self->shm_data->mode = self->mode;
        self->shm_data->head = 0;
        self->shm_data->tail = 0;
        self->shm_data->count = 0;
        self->shm_data->capacity = capacity;
        self->shm_data->item_size = item_size;
    }

    
    self->mutex = CreateMutexA(
        NULL,
        FALSE,
        mutex_name
    );

    if (self->mutex == NULL) {
        if (self->verbose) {
            printf("Failed to create mutex for dispenser '%s': %d\n", self->id, GetLastError());
        }
        UnmapViewOfFile(self->shm_data);
        CloseHandle(self->shm_handle);
        self->shm_data = NULL;
        self->shm_handle = NULL;
        return false;
    }

    
    self->not_empty_semaphore = CreateSemaphoreA(
        NULL,
        0,  // Initial count (0 because it's empty initially)
        LONG_MAX,
        not_empty_name
    );

    if (self->not_empty_semaphore == NULL) {
        if (self->verbose) {
            printf("Failed to create not_empty semaphore for dispenser '%s': %d\n", self->id, GetLastError());
        }
        CloseHandle(self->mutex);
        UnmapViewOfFile(self->shm_data);
        CloseHandle(self->shm_handle);
        self->mutex = NULL;
        self->shm_data = NULL;
        self->shm_handle = NULL;
        return false;
    }

    
    self->not_full_semaphore = CreateSemaphoreA(
        NULL,
        (LONG)capacity,  // Initial count (capacity because it's empty initially)
        LONG_MAX,
        not_full_name
    );

    if (self->not_full_semaphore == NULL) {
        if (self->verbose) {
            printf("Failed to create not_full semaphore for dispenser '%s': %d\n", self->id, GetLastError());
        }
        CloseHandle(self->not_empty_semaphore);
        CloseHandle(self->mutex);
        UnmapViewOfFile(self->shm_data);
        CloseHandle(self->shm_handle);
        self->not_empty_semaphore = NULL;
        self->mutex = NULL;
        self->shm_data = NULL;
        self->shm_handle = NULL;
        return false;
    }

    if (self->verbose) {
        printf("Shared memory dispenser '%s' set up successfully\n", self->id);
    }

    return true;
}

// Add method
bool ShmDispenserPattern_add(ShmDispenserPattern* self, const void* item, size_t item_size) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return false;
    }

    if (item_size > self->shm_data->item_size) {
        if (self->verbose) {
            printf("Error: Item size %zu exceeds maximum size %zu\n", item_size, self->shm_data->item_size);
        }
        return false;
    }

    // Wait for not_full semaphore
    DWORD wait_result = WaitForSingleObject(self->not_full_semaphore, 1000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' is full or timed out\n", self->id);
        }
        return false;
    }

    // Acquire mutex
    wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        ReleaseSemaphore(self->not_full_semaphore, 1, NULL);
        return false;
    }

    
    size_t index;
    if (self->shm_data->mode == SHM_DISPENSER_MODE_FIFO) {
        // FIFO: Add at tail
        index = self->shm_data->tail;
        self->shm_data->tail = (self->shm_data->tail + 1) % self->shm_data->capacity;
    } else {
        // LIFO/DEQUE: Add at head
        self->shm_data->head = (self->shm_data->head == 0) ? 
            self->shm_data->capacity - 1 : self->shm_data->head - 1;
        index = self->shm_data->head;
    }

    set_item(self->shm_data, index, item, item_size);
    self->shm_data->count++;

    if (self->verbose) {
        printf("Added item to dispenser '%s', new count: %zu\n", self->id, self->shm_data->count);
    }

    
    ReleaseMutex(self->mutex);

    
    ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);

    return true;
}

// Add front method
bool ShmDispenserPattern_add_front(ShmDispenserPattern* self, const void* item, size_t item_size) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return false;
    }

    if (item_size > self->shm_data->item_size) {
        if (self->verbose) {
            printf("Error: Item size %zu exceeds maximum size %zu\n", item_size, self->shm_data->item_size);
        }
        return false;
    }

    
    if (self->shm_data->mode != SHM_DISPENSER_MODE_DEQUE) {
        if (self->verbose) {
            printf("Error: add_front only supported in DEQUE mode\n");
        }
        return false;
    }

    // Wait for not_full semaphore
    DWORD wait_result = WaitForSingleObject(self->not_full_semaphore, 1000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' is full or timed out\n", self->id);
        }
        return false;
    }

    // Acquire mutex
    wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        ReleaseSemaphore(self->not_full_semaphore, 1, NULL);
        return false;
    }

    
    set_item(self->shm_data, self->shm_data->tail, item, item_size);
    self->shm_data->tail = (self->shm_data->tail + 1) % self->shm_data->capacity;
    self->shm_data->count++;

    if (self->verbose) {
        printf("Added item to front of dispenser '%s', new count: %zu\n", self->id, self->shm_data->count);
    }

    
    ReleaseMutex(self->mutex);

    
    ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);

    return true;
}

// Add string method
bool ShmDispenserPattern_add_string(ShmDispenserPattern* self, const char* item) {
    return self->add(self, item, strlen(item) + 1);
}

// Add string front method
bool ShmDispenserPattern_add_string_front(ShmDispenserPattern* self, const char* item) {
    return self->add_front(self, item, strlen(item) + 1);
}

// Dispense method
void* ShmDispenserPattern_dispense(ShmDispenserPattern* self, size_t* out_size) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return NULL;
    }

    
    DWORD wait_result = WaitForSingleObject(self->not_empty_semaphore, 1000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' is empty or timed out\n", self->id);
        }
        return NULL;
    }

    // Acquire mutex
    wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    
    size_t index;
    if (self->shm_data->mode == SHM_DISPENSER_MODE_FIFO) {
        
        index = self->shm_data->head;
        self->shm_data->head = (self->shm_data->head + 1) % self->shm_data->capacity;
    } else {
        // LIFO/DEQUE: Get from head
        index = self->shm_data->head;
        self->shm_data->head = (self->shm_data->head + 1) % self->shm_data->capacity;
    }

    size_t item_size = get_item_size(self->shm_data, index);
    void* item_data = get_item_data(self->shm_data, index);

    // Make a copy of the data
    void* data_copy = malloc(item_size);
    if (!data_copy) {
        if (self->verbose) {
            printf("Error: Failed to allocate memory for item\n");
        }
        ReleaseMutex(self->mutex);
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    memcpy(data_copy, item_data, item_size);
    self->shm_data->count--;

    if (self->verbose) {
        printf("Dispensed item from dispenser '%s', new count: %zu\n", self->id, self->shm_data->count);
    }

    if (out_size) {
        *out_size = item_size;
    }

    // Release mutex
    ReleaseMutex(self->mutex);

    // Signal not_full semaphore
    ReleaseSemaphore(self->not_full_semaphore, 1, NULL);

    return data_copy;
}

// Dispense back method
void* ShmDispenserPattern_dispense_back(ShmDispenserPattern* self, size_t* out_size) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return NULL;
    }

    
    if (self->shm_data->mode != SHM_DISPENSER_MODE_DEQUE) {
        if (self->verbose) {
            printf("Error: dispense_back only supported in DEQUE mode\n");
        }
        return NULL;
    }

    
    DWORD wait_result = WaitForSingleObject(self->not_empty_semaphore, 1000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' is empty or timed out\n", self->id);
        }
        return NULL;
    }

    // Acquire mutex
    wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    // Get item from tail
    self->shm_data->tail = (self->shm_data->tail == 0) ? 
        self->shm_data->capacity - 1 : self->shm_data->tail - 1;
    size_t index = self->shm_data->tail;

    size_t item_size = get_item_size(self->shm_data, index);
    void* item_data = get_item_data(self->shm_data, index);

    
    void* data_copy = malloc(item_size);
    if (!data_copy) {
        if (self->verbose) {
            printf("Error: Failed to allocate memory for item\n");
        }
        ReleaseMutex(self->mutex);
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    memcpy(data_copy, item_data, item_size);
    self->shm_data->count--;

    if (self->verbose) {
        printf("Dispensed item from back of dispenser '%s', new count: %zu\n", self->id, self->shm_data->count);
    }

    if (out_size) {
        *out_size = item_size;
    }

    // Release mutex
    ReleaseMutex(self->mutex);

    
    ReleaseSemaphore(self->not_full_semaphore, 1, NULL);

    return data_copy;
}

// Dispense string method
char* ShmDispenserPattern_dispense_string(ShmDispenserPattern* self) {
    size_t size;
    return (char*)self->dispense(self, &size);
}


char* ShmDispenserPattern_dispense_string_back(ShmDispenserPattern* self) {
    size_t size;
    return (char*)self->dispense_back(self, &size);
}

// Peek method
void* ShmDispenserPattern_peek(ShmDispenserPattern* self, size_t* out_size) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return NULL;
    }

    // Wait for not_empty semaphore
    DWORD wait_result = WaitForSingleObject(self->not_empty_semaphore, 1000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' is empty or timed out\n", self->id);
        }
        return NULL;
    }

    // Acquire mutex
    wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    // Get item without removing it
    size_t index = self->shm_data->head;
    size_t item_size = get_item_size(self->shm_data, index);
    void* item_data = get_item_data(self->shm_data, index);

    
    void* data_copy = malloc(item_size);
    if (!data_copy) {
        if (self->verbose) {
            printf("Error: Failed to allocate memory for item\n");
        }
        ReleaseMutex(self->mutex);
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    memcpy(data_copy, item_data, item_size);

    if (self->verbose) {
        printf("Peeked at item from dispenser '%s'\n", self->id);
    }

    if (out_size) {
        *out_size = item_size;
    }

    // Release mutex
    ReleaseMutex(self->mutex);

    
    ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);

    return data_copy;
}


void* ShmDispenserPattern_peek_back(ShmDispenserPattern* self, size_t* out_size) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return NULL;
    }

    
    if (self->shm_data->mode != SHM_DISPENSER_MODE_DEQUE) {
        if (self->verbose) {
            printf("Error: peek_back only supported in DEQUE mode\n");
        }
        return NULL;
    }

    
    DWORD wait_result = WaitForSingleObject(self->not_empty_semaphore, 1000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' is empty or timed out\n", self->id);
        }
        return NULL;
    }

    
    wait_result = WaitForSingleObject(self->mutex, INFINITE);  
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    // Get item from tail without removing it
    size_t index = (self->shm_data->tail == 0) ? 
        self->shm_data->capacity - 1 : self->shm_data->tail - 1;
    
    size_t item_size = get_item_size(self->shm_data, index);
    void* item_data = get_item_data(self->shm_data, index);

    
    void* data_copy = malloc(item_size);
    if (!data_copy) {
        if (self->verbose) {
            printf("Error: Failed to allocate memory for item\n");
        }
        ReleaseMutex(self->mutex);
        ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);
        return NULL;
    }

    memcpy(data_copy, item_data, item_size);

    if (self->verbose) {
        printf("Peeked at item from back of dispenser '%s'\n", self->id);
    }

    if (out_size) {
        *out_size = item_size;
    }

    
    ReleaseMutex(self->mutex);

    // Signal not_empty semaphore again since we didn't remove the item
    ReleaseSemaphore(self->not_empty_semaphore, 1, NULL);

    return data_copy;
}


char* ShmDispenserPattern_peek_string(ShmDispenserPattern* self) {
    size_t size;
    return (char*)self->peek(self, &size);
}

// Peek string back method
char* ShmDispenserPattern_peek_string_back(ShmDispenserPattern* self) {
    size_t size;
    return (char*)self->peek_back(self, &size);
}


bool ShmDispenserPattern_is_empty(ShmDispenserPattern* self) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return true;
    }

    
    DWORD wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        return true;
    }

    bool is_empty = (self->shm_data->count == 0);

    
    ReleaseMutex(self->mutex);

    return is_empty;
}


bool ShmDispenserPattern_is_full(ShmDispenserPattern* self) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return true;
    }

    
    DWORD wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        return true;
    }

    bool is_full = (self->shm_data->count == self->shm_data->capacity);

    // Release mutex
    ReleaseMutex(self->mutex);

    return is_full;
}


void ShmDispenserPattern_clear(ShmDispenserPattern* self) {
    if (!self->shm_data) {
        if (self->verbose) {
            printf("Error: Dispenser '%s' not set up\n", self->id);
        }
        return;
    }

    // Acquire mutex
    DWORD wait_result = WaitForSingleObject(self->mutex, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("Error: Failed to acquire mutex for dispenser '%s'\n", self->id);
        }
        return;
    }

    
    self->shm_data->head = 0;
    self->shm_data->tail = 0;
    size_t old_count = self->shm_data->count;
    self->shm_data->count = 0;

    if (self->verbose) {
        printf("Cleared dispenser '%s'\n", self->id);
    }

    
    ReleaseMutex(self->mutex);

    // Reset semaphores
    
    for (size_t i = 0; i < old_count; i++) {
        WaitForSingleObject(self->not_empty_semaphore, 0);
    }

    
    ReleaseSemaphore(self->not_full_semaphore, (LONG)self->shm_data->capacity, NULL);
}

// Close method
void ShmDispenserPattern_close(ShmDispenserPattern* self) {
    if (self->verbose) {
        printf("Closing shared memory dispenser '%s'\n", self->id);
    }

    
    if (self->not_full_semaphore) {
        CloseHandle(self->not_full_semaphore);
        self->not_full_semaphore = NULL;
    }

    if (self->not_empty_semaphore) {
        CloseHandle(self->not_empty_semaphore);
        self->not_empty_semaphore = NULL;
    }

    if (self->mutex) {
        CloseHandle(self->mutex);
        self->mutex = NULL;
    }

    if (self->shm_data) {
        UnmapViewOfFile(self->shm_data);
        self->shm_data = NULL;
    }

    if (self->shm_handle) {
        CloseHandle(self->shm_handle);
        self->shm_handle = NULL;
    }

    free(self->id);
    self->id = NULL;

    if (self->verbose) {
        printf("Shared memory dispenser closed\n");
    }
}
