// shared_memory.go
package cross_ipc

import (
	"fmt"
	"syscall"
	"unsafe"
)


type SharedMemory struct {
	handle     uintptr
	create     *syscall.Proc
	setup      *syscall.Proc
	write      *syscall.Proc
	writeBytes *syscall.Proc
	read       *syscall.Proc
	readBytes  *syscall.Proc
	clear      *syscall.Proc
	close      *syscall.Proc
	unlink     *syscall.Proc
	destroy    *syscall.Proc
	isLocked   *syscall.Proc // New procedure for checking lock status
}


func NewSharedMemory(id string, size int, verbose bool) (*SharedMemory, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	
	createProc, err := dll.FindProc("SharedMemory_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_create: %w", err)
	}

	setupProc, err := dll.FindProc("SharedMemory_setup_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_setup_api: %w", err)
	}

	writeProc, err := dll.FindProc("SharedMemory_write_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_write_api: %w", err)
	}

	writeBytesProc, err := dll.FindProc("SharedMemory_write_bytes_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_write_bytes_api: %w", err)
	}

	readProc, err := dll.FindProc("SharedMemory_read_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_read_api: %w", err)
	}

	readBytesProc, err := dll.FindProc("SharedMemory_read_bytes_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_read_bytes_api: %w", err)
	}

	clearProc, err := dll.FindProc("SharedMemory_clear_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_clear_api: %w", err)
	}

	closeProc, err := dll.FindProc("SharedMemory_close_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_close_api: %w", err)
	}

	unlinkProc, err := dll.FindProc("SharedMemory_unlink_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_unlink_api: %w", err)
	}

	destroyProc, err := dll.FindProc("SharedMemory_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find SharedMemory_destroy: %w", err)
	}

	
	isLockedProc, err := dll.FindProc("SharedMemory_is_locked")
	if err != nil {
		
		isLockedProc = nil
	}

	
	idBytes := stringToBytes(id)
	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	handle, _, err := createProc.Call(
		uintptr(unsafe.Pointer(&idBytes[0])),
		uintptr(size),
		uintptr(verboseInt),
	)

	if handle == 0 {
		return nil, fmt.Errorf("failed to create SharedMemory: %w", err)
	}

	return &SharedMemory{
		handle:     handle,
		create:     createProc,
		setup:      setupProc,
		write:      writeProc,
		writeBytes: writeBytesProc,
		read:       readProc,
		readBytes:  readBytesProc,
		clear:      clearProc,
		close:      closeProc,
		unlink:     unlinkProc,
		destroy:    destroyProc,
		isLocked:   isLockedProc,
	}, nil
}


func (s *SharedMemory) Setup() (bool, error) {
	result, _, err := s.setup.Call(s.handle)
	return result != 0, err
}


func (s *SharedMemory) Write(data string) error {
	dataBytes := stringToBytes(data)

	_, _, err := s.write.Call(
		s.handle,
		uintptr(unsafe.Pointer(&dataBytes[0])),
	)

	return err
}

// WriteBytes writes bytes to shared memory
func (s *SharedMemory) WriteBytes(data []byte) error {
	if len(data) == 0 {
		return nil
	}

	_, _, err := s.writeBytes.Call(
		s.handle,
		uintptr(unsafe.Pointer(&data[0])),
		uintptr(len(data)),
	)

	return err
}

// Read reads a string from shared memory
func (s *SharedMemory) Read() (string, error) {
	dataPtr, _, err := s.read.Call(s.handle)

	if dataPtr == 0 {
		return "", fmt.Errorf("failed to read from shared memory: %w", err)
	}

	return ptrToString(dataPtr), nil
}

// ReadBytes reads bytes from shared memory
func (s *SharedMemory) ReadBytes() ([]byte, error) {
	var size uintptr
	sizePtr := unsafe.Pointer(&size)

	dataPtr, _, err := s.readBytes.Call(
		s.handle,
		uintptr(sizePtr),
	)

	if dataPtr == 0 || size == 0 {
		return nil, fmt.Errorf("failed to read bytes from shared memory: %w", err)
	}

	// Copy the data to a Go slice
	data := make([]byte, size)
	for i := uintptr(0); i < size; i++ {
		data[i] = *(*byte)(unsafe.Pointer(dataPtr + i))
	}

	return data, nil
}


func (s *SharedMemory) IsLocked() (bool, error) {
	
	if s.isLocked == nil {
		return false, fmt.Errorf("IsLocked function not available in the DLL")
	}

	ret, _, err := s.isLocked.Call(s.handle)
	return ret != 0, err
}


func (s *SharedMemory) Clear() error {
	_, _, err := s.clear.Call(s.handle)
	return err
}

// Close closes the shared memory
func (s *SharedMemory) Close() error {
	if s.handle != 0 {
		_, _, err := s.close.Call(s.handle)
		if err != nil {
			return fmt.Errorf("failed to close shared memory: %w", err)
		}

		_, _, err = s.destroy.Call(s.handle)
		if err != nil {
			return fmt.Errorf("failed to destroy shared memory: %w", err)
		}

		s.handle = 0
	}

	return nil
}


func (s *SharedMemory) Unlink() error {
	if s.handle != 0 {
		_, _, err := s.unlink.Call(s.handle)
		return err
	}
	return nil
}
