// store_dict_pattern.go
package cross_ipc

import (
	"fmt"
	"syscall"
	"unsafe"
)

// StoreDictPattern represents a dictionary-like pattern for storing and retrieving data
type StoreDictPattern struct {
	handle   uintptr
	create   *syscall.Proc
	setup    *syscall.Proc
	store    *syscall.Proc
	retrieve *syscall.Proc
	close    *syscall.Proc
	destroy  *syscall.Proc
}


func NewStoreDictPattern(name string, size int, verbose bool) (*StoreDictPattern, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	
	createProc, err := dll.FindProc("StoreDictPattern_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find StoreDictPattern_create: %w", err)
	}

	setupProc, err := dll.FindProc("StoreDictPattern_setup_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find StoreDictPattern_setup_api: %w", err)
	}

	storeProc, err := dll.FindProc("StoreDictPattern_store_string_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find StoreDictPattern_store_string_api: %w", err)
	}

	retrieveProc, err := dll.FindProc("StoreDictPattern_retrieve_string_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find StoreDictPattern_retrieve_string_api: %w", err)
	}

	closeProc, err := dll.FindProc("StoreDictPattern_close_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find StoreDictPattern_close_api: %w", err)
	}

	destroyProc, err := dll.FindProc("StoreDictPattern_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find StoreDictPattern_destroy: %w", err)
	}

	// Create a StoreDictPattern
	nameBytes := stringToBytes(name)
	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	handle, _, err := createProc.Call(
		uintptr(unsafe.Pointer(&nameBytes[0])),
		uintptr(size),
		uintptr(verboseInt),
	)

	if handle == 0 {
		return nil, fmt.Errorf("failed to create StoreDictPattern: %w", err)
	}

	return &StoreDictPattern{
		handle:   handle,
		create:   createProc,
		setup:    setupProc,
		store:    storeProc,
		retrieve: retrieveProc,
		close:    closeProc,
		destroy:  destroyProc,
	}, nil
}


func (d *StoreDictPattern) Setup() (bool, error) {
	result, _, err := d.setup.Call(d.handle)
	return result != 0, err
}


func (d *StoreDictPattern) Store(key, value string) error {
	keyBytes := stringToBytes(key)
	valueBytes := stringToBytes(value)

	_, _, err := d.store.Call(
		d.handle,
		uintptr(unsafe.Pointer(&keyBytes[0])),
		uintptr(unsafe.Pointer(&valueBytes[0])),
	)

	return handleWindowsError(err)
}

// Retrieve retrieves a value from the dictionary by key
func (d *StoreDictPattern) Retrieve(key string) (string, error) {
	keyBytes := stringToBytes(key)

	valuePtr, _, err := d.retrieve.Call(
		d.handle,
		uintptr(unsafe.Pointer(&keyBytes[0])),
	)

	if valuePtr == 0 {
		return "", fmt.Errorf("failed to retrieve value: %w", err)
	}

	return ptrToString(valuePtr), nil
}


func (d *StoreDictPattern) Close() error {
	if d.handle != 0 {
		_, _, err := d.close.Call(d.handle)
		if err != nil {
			return fmt.Errorf("failed to close StoreDictPattern: %w", err)
		}

		_, _, err = d.destroy.Call(d.handle)
		if err != nil {
			return fmt.Errorf("failed to destroy StoreDictPattern: %w", err)
		}

		d.handle = 0
	}

	return nil
}
