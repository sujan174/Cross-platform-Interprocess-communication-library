#pragma once

#include "cross_ipc.hpp"
#include <vector>

namespace cross_ipc {

class SharedMemory {
public:
    // Constructor and destructor
    SharedMemory(const std::string& id, size_t size, bool verbose = false);
    ~SharedMemory();
    
    // Disable copy and move
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&&) = delete;
    SharedMemory& operator=(SharedMemory&&) = delete;
    
    // Public methods
    bool Setup();
    void Write(const std::string& data);
    void WriteBytes(const std::vector<uint8_t>& data);
    std::string Read();
    std::vector<uint8_t> ReadBytes();
    void Clear();
    void Close();
    void Unlink();
    
    // Lock-related methods
    bool LockForWriting(unsigned long timeout_ms = 5000);
    bool UnlockFromWriting();
    bool WriteWithLock(const std::string& data, unsigned long timeout_ms = 5000);
    bool WriteBytesWithLock(const std::vector<uint8_t>& data, unsigned long timeout_ms = 5000);
    
private:
    void* handle_;
    
    // Function pointers to DLL functions
    using CreateFn = void* (*)(const char*, size_t, bool);
    using SetupFn = bool (*)(void*);
    using WriteFn = void (*)(void*, const char*);
    using WriteBytesFn = void (*)(void*, const uint8_t*, size_t);
    using ReadFn = const char* (*)(void*);
    using ReadBytesFn = const uint8_t* (*)(void*, size_t*);
    using ClearFn = void (*)(void*);
    using CloseFn = void (*)(void*);
    using UnlinkFn = void (*)(void*);
    using DestroyFn = void (*)(void*);
    using LockForWritingFn = bool (*)(void*, unsigned long);
    using UnlockFromWritingFn = bool (*)(void*);
    
    CreateFn create_;
    SetupFn setup_;
    WriteFn write_;
    WriteBytesFn writeBytes_;
    ReadFn read_;
    ReadBytesFn readBytes_;
    ClearFn clear_;
    CloseFn close_;
    UnlinkFn unlink_;
    DestroyFn destroy_;
    LockForWritingFn lockForWriting_;
    UnlockFromWritingFn unlockFromWriting_;
};

} // namespace cross_ipc 