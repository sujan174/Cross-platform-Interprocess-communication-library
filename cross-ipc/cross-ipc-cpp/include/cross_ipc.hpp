#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <windows.h>

namespace cross_ipc {

// ShmDispenserMode represents the mode of operation for a shared memory dispenser
enum class ShmDispenserMode {
    FIFO = 0,  // First In, First Out (queue)
    LIFO = 1,  // Last In, First Out (stack)
    DEQUE = 2  // Double-ended queue (can add/remove from both ends)
};

// Exception class for cross-ipc errors
class CrossIPCError : public std::runtime_error {
public:
    explicit CrossIPCError(const std::string& message) : std::runtime_error(message) {}
};

// DLL management functions
void SetDLLPath(const std::string& path);
HMODULE LoadDLL();

// Helper functions
std::string GetLastErrorAsString();
std::string PtrToString(const char* ptr);

// Forward declarations of all classes
class NamedPipe;
class OrdinaryPipe;
class SharedMemory;
class StoreDictPattern;
class ReqRespPattern;
class PubSubPattern;
class ShmDispenserPattern;

} // namespace cross_ipc 