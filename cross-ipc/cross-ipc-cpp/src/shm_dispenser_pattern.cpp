#include "shm_dispenser_pattern.hpp"

namespace cross_ipc {

ShmDispenserPattern::ShmDispenserPattern(const std::string& name, ShmDispenserMode mode, bool verbose)
    : handle_(nullptr) {
    
    HMODULE dll = LoadDLL();
    
    
    create_ = reinterpret_cast<CreateFn>(GetProcAddress(dll, "ShmDispenserPattern_create"));
    if (!create_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_create: " + GetLastErrorAsString());
    }
    
    setup_ = reinterpret_cast<SetupFn>(GetProcAddress(dll, "ShmDispenserPattern_setup_api"));
    if (!setup_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_setup_api: " + GetLastErrorAsString());
    }
    
    add_ = reinterpret_cast<AddFn>(GetProcAddress(dll, "ShmDispenserPattern_add_string_api"));
    if (!add_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_add_string_api: " + GetLastErrorAsString());
    }
    
    addFront_ = reinterpret_cast<AddFrontFn>(GetProcAddress(dll, "ShmDispenserPattern_add_string_front_api"));
    if (!addFront_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_add_string_front_api: " + GetLastErrorAsString());
    }
    
    dispense_ = reinterpret_cast<DispenseFn>(GetProcAddress(dll, "ShmDispenserPattern_dispense_string_api"));
    if (!dispense_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_dispense_string_api: " + GetLastErrorAsString());
    }
    
    dispenseBack_ = reinterpret_cast<DispenseBackFn>(GetProcAddress(dll, "ShmDispenserPattern_dispense_string_back_api"));
    if (!dispenseBack_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_dispense_string_back_api: " + GetLastErrorAsString());
    }
    
    peek_ = reinterpret_cast<PeekFn>(GetProcAddress(dll, "ShmDispenserPattern_peek_string_api"));
    if (!peek_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_peek_string_api: " + GetLastErrorAsString());
    }
    
    peekBack_ = reinterpret_cast<PeekBackFn>(GetProcAddress(dll, "ShmDispenserPattern_peek_string_back_api"));
    if (!peekBack_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_peek_string_back_api: " + GetLastErrorAsString());
    }
    
    isEmpty_ = reinterpret_cast<IsEmptyFn>(GetProcAddress(dll, "ShmDispenserPattern_is_empty_api"));
    if (!isEmpty_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_is_empty_api: " + GetLastErrorAsString());
    }
    
    isFull_ = reinterpret_cast<IsFullFn>(GetProcAddress(dll, "ShmDispenserPattern_is_full_api"));
    if (!isFull_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_is_full_api: " + GetLastErrorAsString());
    }
    
    clear_ = reinterpret_cast<ClearFn>(GetProcAddress(dll, "ShmDispenserPattern_clear_api"));
    if (!clear_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_clear_api: " + GetLastErrorAsString());
    }
    
    close_ = reinterpret_cast<CloseFn>(GetProcAddress(dll, "ShmDispenserPattern_close_api"));
    if (!close_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_close_api: " + GetLastErrorAsString());
    }
    
    destroy_ = reinterpret_cast<DestroyFn>(GetProcAddress(dll, "ShmDispenserPattern_destroy"));
    if (!destroy_) {
        throw CrossIPCError("Failed to find ShmDispenserPattern_destroy: " + GetLastErrorAsString());
    }
    
    
    handle_ = create_(name.c_str(), static_cast<int>(mode), verbose);
    if (!handle_) {
        throw CrossIPCError("Failed to create ShmDispenserPattern");
    }
}

ShmDispenserPattern::~ShmDispenserPattern() {
    if (handle_) {
        try {
            Close();
        } catch (const std::exception& e) {
            // Log error but don't throw from destructor
            fprintf(stderr, "Error in ShmDispenserPattern destructor: %s\n", e.what());
        }
    }
}

bool ShmDispenserPattern::Setup(int capacity, int itemSize) {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    return setup_(handle_, capacity, itemSize);
}

bool ShmDispenserPattern::Add(const std::string& item) {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    return add_(handle_, item.c_str());
}

bool ShmDispenserPattern::AddFront(const std::string& item) {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    return addFront_(handle_, item.c_str());
}

std::string ShmDispenserPattern::Dispense() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    const char* item = dispense_(handle_);
    if (!item) {
        throw CrossIPCError("Failed to dispense item (dispenser might be empty)");
    }
    
    return PtrToString(item);
}

std::string ShmDispenserPattern::DispenseBack() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    const char* item = dispenseBack_(handle_);
    if (!item) {
        throw CrossIPCError("Failed to dispense item from back (dispenser might be empty)");
    }
    
    return PtrToString(item);
}

std::string ShmDispenserPattern::Peek() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    const char* item = peek_(handle_);
    if (!item) {
        throw CrossIPCError("Failed to peek item (dispenser might be empty)");
    }
    
    return PtrToString(item);
}

std::string ShmDispenserPattern::PeekBack() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    const char* item = peekBack_(handle_);
    if (!item) {
        throw CrossIPCError("Failed to peek item from back (dispenser might be empty)");
    }
    
    return PtrToString(item);
}

bool ShmDispenserPattern::IsEmpty() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    return isEmpty_(handle_);
}

bool ShmDispenserPattern::IsFull() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    return isFull_(handle_);
}

void ShmDispenserPattern::Clear() {
    if (!handle_) {
        throw CrossIPCError("ShmDispenserPattern not initialized");
    }
    
    clear_(handle_);
}

void ShmDispenserPattern::Close() {
    if (handle_) {
        close_(handle_);
        destroy_(handle_);
        handle_ = nullptr;
    }
}

} // namespace cross_ipc 