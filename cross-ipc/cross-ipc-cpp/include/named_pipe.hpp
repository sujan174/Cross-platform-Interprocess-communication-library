#pragma once

#include "cross_ipc.hpp"
#include <functional>

namespace cross_ipc {

class NamedPipe {
public:
    // Constructor and destructor
    NamedPipe(const std::string& pipeName, bool verbose = false);
    ~NamedPipe();
    
    // Disable copy and move
    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe&) = delete;
    NamedPipe(NamedPipe&&) = delete;
    NamedPipe& operator=(NamedPipe&&) = delete;
    
    // Public methods
    void CreatePipe();
    void ConnectPipe();
    void SendMessage(const std::string& message);
    std::string ReceiveMessage();
    void Close();
    
private:
    void* handle_;
    
    // Function pointers to DLL functions
    using CreateFn = void* (*)(const char*, bool);
    using CreatePipeFn = void (*)(void*);
    using ConnectPipeFn = void (*)(void*);
    using SendMessageFn = void (*)(void*, const char*);
    using ReceiveMessageFn = const char* (*)(void*);
    using ClosePipeFn = void (*)(void*);
    using DestroyFn = void (*)(void*);
    
    CreateFn create_;
    CreatePipeFn createPipe_;
    ConnectPipeFn connectPipe_;
    SendMessageFn sendMessage_;
    ReceiveMessageFn receiveMessage_;
    ClosePipeFn closePipe_;
    DestroyFn destroy_;
};

} // namespace cross_ipc 