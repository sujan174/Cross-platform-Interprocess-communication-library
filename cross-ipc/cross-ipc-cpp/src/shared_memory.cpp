#include "shared_memory.hpp"

namespace cross_ipc {

SharedMemory::SharedMemory(const std::string& id, size_t size, bool verbose)
    : handle_(nullptr) {
    
    HMODULE dll = LoadDLL();
    
    // Get function pointers
    create_ = reinterpret_cast<CreateFn>(GetProcAddress(dll, "SharedMemory_create"));
    if (!create_) {
        throw CrossIPCError("Failed to find SharedMemory_create: " + GetLastErrorAsString());
    }
    
    setup_ = reinterpret_cast<SetupFn>(GetProcAddress(dll, "SharedMemory_setup_api"));
    if (!setup_) {
        throw CrossIPCError("Failed to find SharedMemory_setup_api: " + GetLastErrorAsString());
    }
    
    write_ = reinterpret_cast<WriteFn>(GetProcAddress(dll, "SharedMemory_write_api"));
    if (!write_) {
        throw CrossIPCError("Failed to find SharedMemory_write_api: " + GetLastErrorAsString());
    }
    
    writeBytes_ = reinterpret_cast<WriteBytesFn>(GetProcAddress(dll, "SharedMemory_write_bytes_api"));
    if (!writeBytes_) {
        throw CrossIPCError("Failed to find SharedMemory_write_bytes_api: " + GetLastErrorAsString());
    }
    
    read_ = reinterpret_cast<ReadFn>(GetProcAddress(dll, "SharedMemory_read_api"));
    if (!read_) {
        throw CrossIPCError("Failed to find SharedMemory_read_api: " + GetLastErrorAsString());
    }
    
    readBytes_ = reinterpret_cast<ReadBytesFn>(GetProcAddress(dll, "SharedMemory_read_bytes_api"));
    if (!readBytes_) {
        throw CrossIPCError("Failed to find SharedMemory_read_bytes_api: " + GetLastErrorAsString());
    }
    
    clear_ = reinterpret_cast<ClearFn>(GetProcAddress(dll, "SharedMemory_clear_api"));
    if (!clear_) {
        throw CrossIPCError("Failed to find SharedMemory_clear_api: " + GetLastErrorAsString());
    }
    
    close_ = reinterpret_cast<CloseFn>(GetProcAddress(dll, "SharedMemory_close_api"));
    if (!close_) {
        throw CrossIPCError("Failed to find SharedMemory_close_api: " + GetLastErrorAsString());
    }
    
    unlink_ = reinterpret_cast<UnlinkFn>(GetProcAddress(dll, "SharedMemory_unlink_api"));
    if (!unlink_) {
        throw CrossIPCError("Failed to find SharedMemory_unlink_api: " + GetLastErrorAsString());
    }
    
    destroy_ = reinterpret_cast<DestroyFn>(GetProcAddress(dll, "SharedMemory_destroy"));
    if (!destroy_) {
        throw CrossIPCError("Failed to find SharedMemory_destroy: " + GetLastErrorAsString());
    }
    
    // Get lock-related function pointers
    lockForWriting_ = reinterpret_cast<LockForWritingFn>(GetProcAddress(dll, "SharedMemory_lock_for_writing_api"));
    if (!lockForWriting_) {
        throw CrossIPCError("Failed to find SharedMemory_lock_for_writing_api: " + GetLastErrorAsString());
    }
    
    unlockFromWriting_ = reinterpret_cast<UnlockFromWritingFn>(GetProcAddress(dll, "SharedMemory_unlock_from_writing_api"));
    if (!unlockFromWriting_) {
        throw CrossIPCError("Failed to find SharedMemory_unlock_from_writing_api: " + GetLastErrorAsString());
    }
    
    
    handle_ = create_(id.c_str(), size, verbose);
    if (!handle_) {
        throw CrossIPCError("Failed to create SharedMemory");
    }
}

SharedMemory::~SharedMemory() {
    if (handle_) {
        try {
            Close();
        } catch (const std::exception& e) {
            // Log error but don't throw from destructor
            fprintf(stderr, "Error in SharedMemory destructor: %s\n", e.what());
        }
    }
}

bool SharedMemory::Setup() {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    return setup_(handle_);
}

void SharedMemory::Write(const std::string& data) {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    write_(handle_, data.c_str());
}

void SharedMemory::WriteBytes(const std::vector<uint8_t>& data) {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    if (!data.empty()) {
        writeBytes_(handle_, data.data(), data.size());
    }
}

std::string SharedMemory::Read() {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    const char* data = read_(handle_);
    if (!data) {
        throw CrossIPCError("Failed to read from shared memory");
    }
    
    return PtrToString(data);
}

std::vector<uint8_t> SharedMemory::ReadBytes() {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    size_t size = 0;
    const uint8_t* data = readBytes_(handle_, &size);
    
    if (!data || size == 0) {
        throw CrossIPCError("Failed to read bytes from shared memory");
    }
    
    return std::vector<uint8_t>(data, data + size);
}

void SharedMemory::Clear() {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    clear_(handle_);
}

void SharedMemory::Close() {
    if (handle_) {
        close_(handle_);
        destroy_(handle_);
        handle_ = nullptr;
    }
}

void SharedMemory::Unlink() {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    unlink_(handle_);
}


bool SharedMemory::LockForWriting(unsigned long timeout_ms) {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    return lockForWriting_(handle_, timeout_ms);
}

bool SharedMemory::UnlockFromWriting() {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    return unlockFromWriting_(handle_);
}

bool SharedMemory::WriteWithLock(const std::string& data, unsigned long timeout_ms) {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    if (LockForWriting(timeout_ms)) {
        try {
            Write(data);
            UnlockFromWriting();
            return true;
        }
        catch (const std::exception& e) {
            UnlockFromWriting();
            throw;
        }
    }
    return false;
}

bool SharedMemory::WriteBytesWithLock(const std::vector<uint8_t>& data, unsigned long timeout_ms) {
    if (!handle_) {
        throw CrossIPCError("SharedMemory not initialized");
    }
    
    if (LockForWriting(timeout_ms)) {
        try {
            WriteBytes(data);
            UnlockFromWriting();
            return true;
        }
        catch (const std::exception& e) {
            UnlockFromWriting();
            throw;
        }
    }
    return false;
}

} 