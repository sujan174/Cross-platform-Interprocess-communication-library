#include "named_pipe.hpp"

namespace cross_ipc {

NamedPipe::NamedPipe(const std::string& pipeName, bool verbose)
    : handle_(nullptr) {
    
    HMODULE dll = LoadDLL();
    
    
    create_ = reinterpret_cast<CreateFn>(GetProcAddress(dll, "NamedPipe_create"));
    if (!create_) {
        throw CrossIPCError("Failed to find NamedPipe_create: " + GetLastErrorAsString());
    }
    
    createPipe_ = reinterpret_cast<CreatePipeFn>(GetProcAddress(dll, "NamedPipe_create_pipe_api"));
    if (!createPipe_) {
        throw CrossIPCError("Failed to find NamedPipe_create_pipe_api: " + GetLastErrorAsString());
    }
    
    connectPipe_ = reinterpret_cast<ConnectPipeFn>(GetProcAddress(dll, "NamedPipe_connect_pipe_api"));
    if (!connectPipe_) {
        throw CrossIPCError("Failed to find NamedPipe_connect_pipe_api: " + GetLastErrorAsString());
    }
    
    sendMessage_ = reinterpret_cast<SendMessageFn>(GetProcAddress(dll, "NamedPipe_send_message_api"));
    if (!sendMessage_) {
        throw CrossIPCError("Failed to find NamedPipe_send_message_api: " + GetLastErrorAsString());
    }
    
    receiveMessage_ = reinterpret_cast<ReceiveMessageFn>(GetProcAddress(dll, "NamedPipe_receive_message_api"));
    if (!receiveMessage_) {
        throw CrossIPCError("Failed to find NamedPipe_receive_message_api: " + GetLastErrorAsString());
    }
    
    closePipe_ = reinterpret_cast<ClosePipeFn>(GetProcAddress(dll, "NamedPipe_close_pipe_api"));
    if (!closePipe_) {
        throw CrossIPCError("Failed to find NamedPipe_close_pipe_api: " + GetLastErrorAsString());
    }
    
    destroy_ = reinterpret_cast<DestroyFn>(GetProcAddress(dll, "NamedPipe_destroy"));
    if (!destroy_) {
        throw CrossIPCError("Failed to find NamedPipe_destroy: " + GetLastErrorAsString());
    }
    
    
    handle_ = create_(pipeName.c_str(), verbose);
    if (!handle_) {
        throw CrossIPCError("Failed to create NamedPipe");
    }
}

NamedPipe::~NamedPipe() {
    if (handle_) {
        try {
            Close();
        } catch (const std::exception& e) {
            
            fprintf(stderr, "Error in NamedPipe destructor: %s\n", e.what());
        }
    }
}

void NamedPipe::CreatePipe() {
    if (!handle_) {
        throw CrossIPCError("NamedPipe not initialized");
    }
    
    createPipe_(handle_);
}

void NamedPipe::ConnectPipe() {
    if (!handle_) {
        throw CrossIPCError("NamedPipe not initialized");
    }
    
    connectPipe_(handle_);
}

void NamedPipe::SendMessage(const std::string& message) {
    if (!handle_) {
        throw CrossIPCError("NamedPipe not initialized");
    }
    
    sendMessage_(handle_, message.c_str());
}

std::string NamedPipe::ReceiveMessage() {
    if (!handle_) {
        throw CrossIPCError("NamedPipe not initialized");
    }
    
    const char* message = receiveMessage_(handle_);
    if (!message) {
        throw CrossIPCError("Failed to receive message");
    }
    
    return PtrToString(message);
}

void NamedPipe::Close() {
    if (handle_) {
        closePipe_(handle_);
        destroy_(handle_);
        handle_ = nullptr;
    }
}

} // namespace cross_ipc 