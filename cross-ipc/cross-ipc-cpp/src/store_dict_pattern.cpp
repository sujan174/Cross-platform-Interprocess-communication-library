#include "store_dict_pattern.hpp"

namespace cross_ipc {

StoreDictPattern::StoreDictPattern(const std::string& name, size_t size, bool verbose)
    : handle_(nullptr) {
    
    HMODULE dll = LoadDLL();
    
    
    create_ = reinterpret_cast<CreateFn>(GetProcAddress(dll, "StoreDictPattern_create"));
    if (!create_) {
        throw CrossIPCError("Failed to find StoreDictPattern_create: " + GetLastErrorAsString());
    }
    
    setup_ = reinterpret_cast<SetupFn>(GetProcAddress(dll, "StoreDictPattern_setup_api"));
    if (!setup_) {
        throw CrossIPCError("Failed to find StoreDictPattern_setup_api: " + GetLastErrorAsString());
    }
    
    store_ = reinterpret_cast<StoreFn>(GetProcAddress(dll, "StoreDictPattern_store_string_api"));
    if (!store_) {
        throw CrossIPCError("Failed to find StoreDictPattern_store_string_api: " + GetLastErrorAsString());
    }
    
    retrieve_ = reinterpret_cast<RetrieveFn>(GetProcAddress(dll, "StoreDictPattern_retrieve_string_api"));
    if (!retrieve_) {
        throw CrossIPCError("Failed to find StoreDictPattern_retrieve_string_api: " + GetLastErrorAsString());
    }
    
    close_ = reinterpret_cast<CloseFn>(GetProcAddress(dll, "StoreDictPattern_close_api"));
    if (!close_) {
        throw CrossIPCError("Failed to find StoreDictPattern_close_api: " + GetLastErrorAsString());
    }
    
    destroy_ = reinterpret_cast<DestroyFn>(GetProcAddress(dll, "StoreDictPattern_destroy"));
    if (!destroy_) {
        throw CrossIPCError("Failed to find StoreDictPattern_destroy: " + GetLastErrorAsString());
    }
    
    // Create the dictionary
    handle_ = create_(name.c_str(), size, verbose);
    if (!handle_) {
        throw CrossIPCError("Failed to create StoreDictPattern");
    }
}

StoreDictPattern::~StoreDictPattern() {
    if (handle_) {
        try {
            Close();
        } catch (const std::exception& e) {
            // Log error but don't throw from destructor
            fprintf(stderr, "Error in StoreDictPattern destructor: %s\n", e.what());
        }
    }
}

bool StoreDictPattern::Setup() {
    if (!handle_) {
        throw CrossIPCError("StoreDictPattern not initialized");
    }
    
    return setup_(handle_);
}

void StoreDictPattern::Store(const std::string& key, const std::string& value) {
    if (!handle_) {
        throw CrossIPCError("StoreDictPattern not initialized");
    }
    
    store_(handle_, key.c_str(), value.c_str());
}

std::string StoreDictPattern::Retrieve(const std::string& key) {
    if (!handle_) {
        throw CrossIPCError("StoreDictPattern not initialized");
    }
    
    const char* value = retrieve_(handle_, key.c_str());
    if (!value) {
        throw CrossIPCError("Failed to retrieve value for key: " + key);
    }
    
    return PtrToString(value);
}

void StoreDictPattern::Close() {
    if (handle_) {
        close_(handle_);
        destroy_(handle_);
        handle_ = nullptr;
    }
}

} 