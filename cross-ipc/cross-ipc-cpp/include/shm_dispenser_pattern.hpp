#pragma once

#include "cross_ipc.hpp"

namespace cross_ipc {

class ShmDispenserPattern {
public:
    // Constructor and destructor
    ShmDispenserPattern(const std::string& name, ShmDispenserMode mode, bool verbose = false);
    ~ShmDispenserPattern();
    
    // Disable copy and move
    ShmDispenserPattern(const ShmDispenserPattern&) = delete;
    ShmDispenserPattern& operator=(const ShmDispenserPattern&) = delete;
    ShmDispenserPattern(ShmDispenserPattern&&) = delete;
    ShmDispenserPattern& operator=(ShmDispenserPattern&&) = delete;
    
    // Public methods
    bool Setup(int capacity, int itemSize);
    bool Add(const std::string& item);
    bool AddFront(const std::string& item);
    std::string Dispense();
    std::string DispenseBack();
    std::string Peek();
    std::string PeekBack();
    bool IsEmpty();
    bool IsFull();
    void Clear();
    void Close();
    
private:
    void* handle_;
    
    // Function pointers to DLL functions
    using CreateFn = void* (*)(const char*, int, bool);
    using SetupFn = bool (*)(void*, int, int);
    using AddFn = bool (*)(void*, const char*);
    using AddFrontFn = bool (*)(void*, const char*);
    using DispenseFn = const char* (*)(void*);
    using DispenseBackFn = const char* (*)(void*);
    using PeekFn = const char* (*)(void*);
    using PeekBackFn = const char* (*)(void*);
    using IsEmptyFn = bool (*)(void*);
    using IsFullFn = bool (*)(void*);
    using ClearFn = void (*)(void*);
    using CloseFn = void (*)(void*);
    using DestroyFn = void (*)(void*);
    
    CreateFn create_;
    SetupFn setup_;
    AddFn add_;
    AddFrontFn addFront_;
    DispenseFn dispense_;
    DispenseBackFn dispenseBack_;
    PeekFn peek_;
    PeekBackFn peekBack_;
    IsEmptyFn isEmpty_;
    IsFullFn isFull_;
    ClearFn clear_;
    CloseFn close_;
    DestroyFn destroy_;
};

} // namespace cross_ipc 