
package cross_ipc

import (
	"fmt"
	"sync"
	"syscall"
	"unsafe"
)

// ShmDispenserMode represents the mode of operation for a shared memory dispenser
type ShmDispenserMode int

const (
	
	FIFO ShmDispenserMode = 0
	// LIFO mode: Last In, First Out (stack)
	LIFO ShmDispenserMode = 1
	// DEQUE mode: Double-ended queue (can add/remove from both ends)
	DEQUE ShmDispenserMode = 2
)

var (
	dll      *syscall.DLL
	dllMutex sync.Mutex
	dllPath  = "C:/GIthubProjects/cross-ipc/cross-ipc/x64/Debug/cross-ipc.dll" // Default path
)


func SetDLLPath(path string) {
	dllMutex.Lock()
	defer dllMutex.Unlock()
	dllPath = path
}


func loadDLL() (*syscall.DLL, error) {
	dllMutex.Lock()
	defer dllMutex.Unlock()

	if dll != nil {
		return dll, nil
	}

	var err error
	dll, err = syscall.LoadDLL(dllPath)
	if err != nil {
		return nil, fmt.Errorf("failed to load DLL: %w", err)
	}

	return dll, nil
}


func stringToBytes(s string) []byte {
	return []byte(s + "\x00")
}


func ptrToString(ptr uintptr) string {
	if ptr == 0 {
		return ""
	}

	var byteSlice []byte
	for i := 0; ; i++ {
		b := *(*byte)(unsafe.Pointer(ptr + uintptr(i)))
		if b == 0 {
			break
		}
		byteSlice = append(byteSlice, b)
	}

	return string(byteSlice)
}

// Helper function to handle Windows "success" errors
func handleWindowsError(err error) error {
	if err != nil && err.Error() == "The operation completed successfully." {
		return nil
	}
	return err
}
