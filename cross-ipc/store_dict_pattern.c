#include "store_dict_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>

#define INITIAL_CAPACITY 16


static int find_entry_index(StoreDictPattern* dict, const char* key) {
    for (size_t i = 0; i < dict->entry_count; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) {
            return (int)i;
        }
    }
    return -1;
}


static bool resize_entries(StoreDictPattern* dict, size_t new_capacity) {
    if (dict->verbose) {
        printf("resize_entries: Resizing from %zu to %zu\n", dict->entry_capacity, new_capacity);
    }

    if (new_capacity == 0) {
        
        free(dict->entries);
        dict->entries = NULL;
        dict->entry_capacity = 0;
        return true;
    }

    DictEntry* new_entries = NULL;

    if (dict->entries == NULL) {
        
        new_entries = (DictEntry*)malloc(new_capacity * sizeof(DictEntry));
    }
    else {
        // Reallocation
        new_entries = (DictEntry*)realloc(dict->entries, new_capacity * sizeof(DictEntry));
    }

    if (!new_entries) {
        if (dict->verbose) {
            printf("resize_entries: Failed to allocate memory for entries\n");
        }
        return false;
    }

    dict->entries = new_entries;
    dict->entry_capacity = new_capacity;

    if (dict->verbose) {
        printf("resize_entries: Resize successful\n");
    }
    return true;
}

// Constructor implementation
void StoreDictPattern_init(StoreDictPattern* store, const char* id, size_t size, bool verbose) {
    if (verbose) {
        printf("StoreDictPattern_init: Initializing with id '%s' and size %zu\n", id, size);
    }

    
    SharedMemory_init(&store->shm, id, size, verbose);

    
    store->entries = NULL;
    store->entry_count = 0;
    store->entry_capacity = 0;
    store->verbose = verbose;
    store->version = 0;  // Initialize version to 0

    // Create named mutex for synchronization
    char mutex_name[256];
    sprintf_s(mutex_name, sizeof(mutex_name), "StoreDictPattern_Mutex_%s", id);
    store->mutex = CreateMutexA(NULL, FALSE, mutex_name);
    
    if (store->mutex == NULL && verbose) {
        printf("Failed to create mutex: %lu\n", GetLastError());
    }

    
    store->setup = StoreDictPattern_setup;
    store->store = StoreDictPattern_store;
    store->store_string = StoreDictPattern_store_string;
    store->store_bytes = StoreDictPattern_store_bytes;
    store->retrieve = StoreDictPattern_retrieve;
    store->retrieve_bytes = StoreDictPattern_retrieve_bytes;
    store->retrieve_string = StoreDictPattern_retrieve_string;
    store->load = StoreDictPattern_load;
    store->sync = StoreDictPattern_sync;
    store->list_keys = StoreDictPattern_list_keys;
    store->close = StoreDictPattern_close;

    
    if (!resize_entries(store, 10)) {
        if (verbose) {
            printf("StoreDictPattern_init: Failed to allocate initial entries\n");
        }
    }
    else if (verbose) {
        printf("StoreDictPattern_init: Allocated initial entries\n");
    }

    if (verbose) {
        printf("StoreDictPattern_init: Initialization completed\n");
    }
}

bool StoreDictPattern_setup(StoreDictPattern* self) {
    if (self->verbose) {
        printf("StoreDictPattern_setup: Starting setup\n");
    }

    
    bool result = self->shm.setup(&self->shm);
    if (!result) {
        if (self->verbose) {
            printf("StoreDictPattern_setup: Failed to set up shared memory\n");
        }
        return false;
    }

    if (self->verbose) {
        printf("StoreDictPattern_setup: Shared memory set up successfully\n");
        printf("StoreDictPattern_setup: Loading existing data\n");
    }

    // Load existing data if any
    self->load(self);

    if (self->verbose) {
        printf("StoreDictPattern_setup: Setup completed successfully\n");
    }
    return true;
}

void StoreDictPattern_load(StoreDictPattern* self) {
    if (self->verbose) {
        printf("StoreDictPattern_load: Loading entries from shared memory\n");
    }

    
    size_t data_size;
    unsigned char* data = self->shm.read_bytes(&self->shm, &data_size);

    if (!data || data_size == 0) {
        if (self->verbose) {
            printf("StoreDictPattern_load: No data in shared memory\n");
        }
        return;
    }

    // Parse data
    size_t pos = 0;

    
    uint32_t entry_count = *(uint32_t*)(data + pos);
    pos += sizeof(uint32_t);

    if (self->verbose) {
        printf("StoreDictPattern_load: Found %u entries\n", entry_count);
    }

    
    for (size_t i = 0; i < self->entry_count; i++) {
        free(self->entries[i].key);
        free(self->entries[i].value);
    }
    self->entry_count = 0;

    
    if (entry_count > self->entry_capacity) {
        if (!resize_entries(self, entry_count)) {
            printf("StoreDictPattern_load: Failed to resize entries array\n");
            free(data);
            return;
        }
    }

    // Read entries
    for (size_t i = 0; i < entry_count; i++) {
        
        uint32_t key_len = *(uint32_t*)(data + pos);
        pos += sizeof(uint32_t);

        
        char* key = _strdup((char*)(data + pos));
        pos += key_len;

        
        uint32_t value_size = *(uint32_t*)(data + pos);
        pos += sizeof(uint32_t);

        // Read value
        unsigned char* value = (unsigned char*)malloc(value_size);
        memcpy(value, data + pos, value_size);
        pos += value_size;

        // Add entry
        self->entries[self->entry_count].key = key;
        self->entries[self->entry_count].value = value;
        self->entries[self->entry_count].value_size = value_size;
        self->entry_count++;

        if (self->verbose) {
            printf("StoreDictPattern_load: Loaded entry %zu: key='%s', value_size=%u\n",
                i, key, value_size);
        }
    }

    
    free(data);

    if (self->verbose) {
        printf("StoreDictPattern_load: Loaded %zu entries from shared memory\n", self->entry_count);
    }
}

void StoreDictPattern_store_bytes(StoreDictPattern* self, const char* key, const unsigned char* value, size_t value_size) {
    
    StoreDictPattern_store(self, key, value, value_size);
}

void StoreDictPattern_store_string(StoreDictPattern* self, const char* key, const char* value) {
    if (self->verbose) {
        printf("StoreDictPattern_store_string: Storing key '%s'\n", key);
    }

    // Calculate the value size (including null terminator)
    size_t value_size = strlen(value) + 1;

    
    unsigned char* value_copy = (unsigned char*)malloc(value_size);
    if (!value_copy) {
        if (self->verbose) {
            printf("StoreDictPattern_store_string: Failed to allocate memory for value\n");
        }
        return;
    }

    // Copy the value (including null terminator)
    memcpy(value_copy, value, value_size);

    // Store the key-value pair
    self->store(self, key, value_copy, value_size);

    // Free the value copy (store makes its own copy)
    free(value_copy);

    if (self->verbose) {
        printf("Stored key '%s' with %zu bytes\n", key, value_size - 1); // -1 for null terminator
    }
}

unsigned char* StoreDictPattern_retrieve(StoreDictPattern* self, const char* key, size_t* out_size) {
    // Wait for mutex
    DWORD wait_result = WaitForSingleObject(self->mutex, 5000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) printf("Failed to acquire mutex: %lu\n", GetLastError());
        return NULL;
    }
    
    
    self->load(self);
    
    // Find the entry
    unsigned char* result = NULL;
    for (size_t i = 0; i < self->entry_count; i++) {
        if (strcmp(self->entries[i].key, key) == 0) {
            
            result = malloc(self->entries[i].value_size);
            memcpy(result, self->entries[i].value, self->entries[i].value_size);
            
            if (out_size) {
                *out_size = self->entries[i].value_size;
            }
            break;
        }
    }
    
    // Release mutex
    ReleaseMutex(self->mutex);
    return result;
}

unsigned char* StoreDictPattern_retrieve_bytes(StoreDictPattern* self, const char* key, size_t* out_size) {
    
    return StoreDictPattern_retrieve(self, key, out_size);
}

char* StoreDictPattern_retrieve_string(StoreDictPattern* self, const char* key) {
    if (self->verbose) {
        printf("StoreDictPattern_retrieve_string: Retrieving value for key '%s'\n", key);
    }

    // Wait for mutex
    DWORD wait_result = WaitForSingleObject(self->mutex, 5000);
    if (wait_result != WAIT_OBJECT_0) {
        if (self->verbose) {
            printf("StoreDictPattern_retrieve_string: Failed to acquire mutex\n");
        }
        return NULL;
    }

    // Load latest data from shared memory
    self->load(self);

    
    int index = find_entry_index(self, key);
    if (index < 0) {
        if (self->verbose) {
            printf("StoreDictPattern_retrieve_string: Key '%s' not found\n", key);
        }
        ReleaseMutex(self->mutex);
        return NULL;
    }

    
    char* result = _strdup((char*)self->entries[index].value);

    
    ReleaseMutex(self->mutex);

    if (self->verbose) {
        printf("StoreDictPattern_retrieve_string: Retrieved value '%s' for key '%s'\n", 
               result, key);
    }

    return result;
}

void StoreDictPattern_delete(StoreDictPattern* self, const char* key) {
    int index = find_entry_index(self, key);
    if (index < 0) {
        if (self->verbose) {
            printf("Key '%s' not found\n", key);
        }
        return;
    }

    // Free memory
    free(self->entries[index].key);
    free(self->entries[index].value);

    
    if (index < self->entry_count - 1) {
        self->entries[index] = self->entries[self->entry_count - 1];
    }

    self->entry_count--;
    self->sync(self);

    if (self->verbose) {
        printf("Deleted key '%s'\n", key);
    }
}

char** StoreDictPattern_list_keys(StoreDictPattern* self, size_t* out_count) {
    
    

    if (self->entry_count == 0) {
        if (out_count) {
            *out_count = 0;
        }
        return NULL;
    }

    char** keys = (char**)malloc(self->entry_count * sizeof(char*));
    for (size_t i = 0; i < self->entry_count; i++) {
        keys[i] = _strdup(self->entries[i].key);
    }

    if (out_count) {
        *out_count = self->entry_count;
    }

    return keys;
}

void StoreDictPattern_clear(StoreDictPattern* self) {
    
    for (size_t i = 0; i < self->entry_count; i++) {
        free(self->entries[i].key);
        free(self->entries[i].value);
    }

    self->entry_count = 0;

    // Clear shared memory
    self->shm.clear(&self->shm);

    if (self->verbose) {
        printf("Dictionary cleared\n");
    }
}

void StoreDictPattern_close(StoreDictPattern* self) {
    self->shm.close(&self->shm);

    if (self->verbose) {
        printf("Dictionary closed\n");
    }
}

bool StoreDictPattern_sync(StoreDictPattern* self) {
    if (self->verbose) {
        printf("StoreDictPattern_sync: Syncing %zu entries to shared memory\n", self->entry_count);
    }

    
    DWORD wait_result = WaitForSingleObject(self->mutex, 5000);
    if (wait_result != WAIT_OBJECT_0) return false;
    
    // Increment version number
    self->version++;
    
    // Calculate buffer size needed
    size_t buffer_size = sizeof(uint32_t);  // For version
    buffer_size += sizeof(uint32_t);        
    for (size_t i = 0; i < self->entry_count; i++) {
        buffer_size += sizeof(uint32_t); 
        buffer_size += strlen(self->entries[i].key) + 1; 
        buffer_size += sizeof(uint32_t); 
        buffer_size += self->entries[i].value_size; 
    }

    if (buffer_size > self->shm.size) {
        printf("StoreDictPattern_sync: Buffer size %zu exceeds shared memory size %zu\n",
            buffer_size, self->shm.size);
        ReleaseMutex(self->mutex);
        return false;
    }

    
    unsigned char* buffer = (unsigned char*)malloc(buffer_size);
    if (!buffer) {
        printf("StoreDictPattern_sync: Failed to allocate buffer\n");
        ReleaseMutex(self->mutex);
        return false;
    }

    // Fill buffer
    size_t pos = 0;

    // Write version number first
    *(uint32_t*)(buffer + pos) = self->version;
    pos += sizeof(uint32_t);

    
    *(uint32_t*)(buffer + pos) = (uint32_t)self->entry_count;
    pos += sizeof(uint32_t);

    
    for (size_t i = 0; i < self->entry_count; i++) {
        // Write key length
        uint32_t key_len = (uint32_t)strlen(self->entries[i].key) + 1; // Include null terminator
        *(uint32_t*)(buffer + pos) = key_len;
        pos += sizeof(uint32_t);

        
        memcpy(buffer + pos, self->entries[i].key, key_len);
        pos += key_len;

        
        *(uint32_t*)(buffer + pos) = (uint32_t)self->entries[i].value_size;
        pos += sizeof(uint32_t);

        
        memcpy(buffer + pos, self->entries[i].value, self->entries[i].value_size);
        pos += self->entries[i].value_size;
    }

    if (self->verbose) {
        printf("Buffer dump (first 100 bytes or less):\n");
        for (size_t i = 0; i < pos && i < 100; i++) {
            printf("%02X ", buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
    }

    
    bool success = self->shm.write_bytes(&self->shm, buffer, pos);

    // Free buffer
    free(buffer);

    if (!success && self->verbose) {
        printf("StoreDictPattern_sync: Failed to write to shared memory\n");
    }

    if (self->verbose) {
        printf("StoreDictPattern_sync: Synced %zu entries to shared memory (total %zu bytes)\n",
            self->entry_count, pos);
    }

    // Release mutex
    ReleaseMutex(self->mutex);
    return success;
}

void StoreDictPattern_store(StoreDictPattern* self, const char* key, const unsigned char* value, size_t value_size) {
    // Check if the key already exists
    int index = find_entry_index(self, key);

    if (index >= 0) {
        
        free(self->entries[index].value);

        
        unsigned char* value_copy = (unsigned char*)malloc(value_size);
        if (!value_copy) {
            printf("Failed to allocate memory for value\n");
            return;
        }

        memcpy(value_copy, value, value_size);
        self->entries[index].value = value_copy;
        self->entries[index].value_size = value_size;
    }
    else {
        
        if (self->entry_count >= self->entry_capacity) {
            if (!resize_entries(self, self->entry_capacity * 2)) {
                printf("Failed to resize entries array\n");
                return;
            }
        }

        // Make a copy of the key
        char* key_copy = _strdup(key);
        if (!key_copy) {
            printf("Failed to allocate memory for key\n");
            return;
        }

        // Make a copy of the value
        unsigned char* value_copy = (unsigned char*)malloc(value_size);
        if (!value_copy) {
            printf("Failed to allocate memory for value\n");
            free(key_copy);
            return;
        }

        memcpy(value_copy, value, value_size);

        
        self->entries[self->entry_count].key = key_copy;
        self->entries[self->entry_count].value = value_copy;
        self->entries[self->entry_count].value_size = value_size;
        self->entry_count++;
    }

    
    self->sync(self);
}