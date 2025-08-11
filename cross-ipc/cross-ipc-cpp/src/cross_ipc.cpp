#include "cross_ipc.hpp"
#include <mutex>

namespace cross_ipc {


static std::string dllPath = "C:/GIthubProjects/cross-ipc/cross-ipc/x64/Debug/cross-ipc.dll";
static HMODULE dllHandle = nullptr;
static std::mutex dllMutex;

void SetDLLPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(dllMutex);
    dllPath = path;
    
    
    if (dllHandle) {
        FreeLibrary(dllHandle);
        dllHandle = nullptr;
    }
}

HMODULE LoadDLL() {
    std::lock_guard<std::mutex> lock(dllMutex);
    
    if (dllHandle) {
        return dllHandle;
    }
    
    dllHandle = LoadLibraryA(dllPath.c_str());
    if (!dllHandle) {
        throw CrossIPCError("Failed to load DLL: " + GetLastErrorAsString());
    }
    
    return dllHandle;
}

std::string GetLastErrorAsString() {
    DWORD error = GetLastError();
    if (error == 0) {
        return "";
    }
    
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
    
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    
    return message;
}

std::string PtrToString(const char* ptr) {
    if (!ptr) {
        return "";
    }
    return std::string(ptr);
}

} 