#pragma once

#include "cross_ipc.hpp"

namespace cross_ipc {

class StoreDictPattern {
public:
    // Constructor and destructor
    StoreDictPattern(const std::string& name, size_t size, bool verbose = false);
    ~StoreDictPattern();
    
    // Disable copy and move
    StoreDictPattern(const StoreDictPattern&) = delete;
    StoreDictPattern& operator=(const StoreDictPattern&) = delete;
    StoreDictPattern(StoreDictPattern&&) = delete;
    StoreDictPattern& operator=(StoreDictPattern&&) = delete;
    
    // Public methods
    bool Setup();
    void Store(const std::string& key, const std::string& value);
    std::string Retrieve(const std::string& key);
    void Close();
    
private:
    void* handle_;
    
    // Function pointers to DLL functions
    using CreateFn = void* (*)(const char*, size_t, bool);
    using SetupFn = bool (*)(void*);
    using StoreFn = void (*)(void*, const char*, const char*);
    using RetrieveFn = const char* (*)(void*, const char*);
    using CloseFn = void (*)(void*);
    using DestroyFn = void (*)(void*);
    
    CreateFn create_;
    SetupFn setup_;
    StoreFn store_;
    RetrieveFn retrieve_;
    CloseFn close_;
    DestroyFn destroy_;
};

} // namespace cross_ipc 