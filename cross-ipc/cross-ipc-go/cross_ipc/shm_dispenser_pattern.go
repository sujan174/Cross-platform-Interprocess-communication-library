package cross_ipc

import (
	"fmt"
	"syscall"
	"unsafe"
)

// ShmDispenserPattern represents a dispenser pattern for sharing data items between processes
type ShmDispenserPattern struct {
	handle       uintptr
	create       *syscall.Proc
	setup        *syscall.Proc
	add          *syscall.Proc
	addFront     *syscall.Proc
	dispense     *syscall.Proc
	dispenseBack *syscall.Proc
	peek         *syscall.Proc
	peekBack     *syscall.Proc
	isEmpty      *syscall.Proc
	isFull       *syscall.Proc
	clear        *syscall.Proc
	close        *syscall.Proc
	destroy      *syscall.Proc
}


func NewShmDispenserPattern(name string, mode ShmDispenserMode, verbose bool) (*ShmDispenserPattern, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	
	createProc, err := dll.FindProc("ShmDispenserPattern_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_create: %w", err)
	}

	setupProc, err := dll.FindProc("ShmDispenserPattern_setup_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_setup_api: %w", err)
	}

	addProc, err := dll.FindProc("ShmDispenserPattern_add_string_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_add_string_api: %w", err)
	}

	addFrontProc, err := dll.FindProc("ShmDispenserPattern_add_string_front_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_add_string_front_api: %w", err)
	}

	dispenseProc, err := dll.FindProc("ShmDispenserPattern_dispense_string_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_dispense_string_api: %w", err)
	}

	dispenseBackProc, err := dll.FindProc("ShmDispenserPattern_dispense_string_back_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_dispense_string_back_api: %w", err)
	}

	peekProc, err := dll.FindProc("ShmDispenserPattern_peek_string_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_peek_string_api: %w", err)
	}

	peekBackProc, err := dll.FindProc("ShmDispenserPattern_peek_string_back_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_peek_string_back_api: %w", err)
	}

	isEmptyProc, err := dll.FindProc("ShmDispenserPattern_is_empty_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_is_empty_api: %w", err)
	}

	isFullProc, err := dll.FindProc("ShmDispenserPattern_is_full_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_is_full_api: %w", err)
	}

	clearProc, err := dll.FindProc("ShmDispenserPattern_clear_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_clear_api: %w", err)
	}

	closeProc, err := dll.FindProc("ShmDispenserPattern_close_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_close_api: %w", err)
	}

	destroyProc, err := dll.FindProc("ShmDispenserPattern_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find ShmDispenserPattern_destroy: %w", err)
	}

	
	nameBytes := stringToBytes(name)
	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	handle, _, err := createProc.Call(
		uintptr(unsafe.Pointer(&nameBytes[0])),
		uintptr(int(mode)),
		uintptr(verboseInt),
	)

	if handle == 0 {
		return nil, fmt.Errorf("failed to create ShmDispenserPattern: %w", err)
	}

	return &ShmDispenserPattern{
		handle:       handle,
		create:       createProc,
		setup:        setupProc,
		add:          addProc,
		addFront:     addFrontProc,
		dispense:     dispenseProc,
		dispenseBack: dispenseBackProc,
		peek:         peekProc,
		peekBack:     peekBackProc,
		isEmpty:      isEmptyProc,
		isFull:       isFullProc,
		clear:        clearProc,
		close:        closeProc,
		destroy:      destroyProc,
	}, nil
}


func (d *ShmDispenserPattern) Setup(capacity, itemSize int) (bool, error) {
	result, _, err := d.setup.Call(
		d.handle,
		uintptr(capacity),
		uintptr(itemSize),
	)
	return result != 0, err
}

// Add adds an item to the back of the dispenser
func (d *ShmDispenserPattern) Add(item string) (bool, error) {
	itemBytes := stringToBytes(item)

	result, _, err := d.add.Call(
		d.handle,
		uintptr(unsafe.Pointer(&itemBytes[0])),
	)

	return result != 0, err
}

// AddFront adds an item to the front of the dispenser (only in DEQUE mode)
func (d *ShmDispenserPattern) AddFront(item string) (bool, error) {
	itemBytes := stringToBytes(item)

	result, _, err := d.addFront.Call(
		d.handle,
		uintptr(unsafe.Pointer(&itemBytes[0])),
	)

	return result != 0, err
}


func (d *ShmDispenserPattern) Dispense() (string, error) {
	itemPtr, _, err := d.dispense.Call(d.handle)

	if itemPtr == 0 {
		return "", fmt.Errorf("failed to dispense item: %w", err)
	}

	return ptrToString(itemPtr), nil
}


func (d *ShmDispenserPattern) DispenseBack() (string, error) {
	itemPtr, _, err := d.dispenseBack.Call(d.handle)

	if itemPtr == 0 {
		return "", fmt.Errorf("failed to dispense item from back: %w", err)
	}

	return ptrToString(itemPtr), nil
}

// Peek returns the item at the front of the dispenser without removing it
func (d *ShmDispenserPattern) Peek() (string, error) {
	itemPtr, _, err := d.peek.Call(d.handle)

	if itemPtr == 0 {
		return "", fmt.Errorf("failed to peek item: %w", err)
	}

	return ptrToString(itemPtr), nil
}

// PeekBack returns the item at the back of the dispenser without removing it (only in DEQUE mode)
func (d *ShmDispenserPattern) PeekBack() (string, error) {
	itemPtr, _, err := d.peekBack.Call(d.handle)

	if itemPtr == 0 {
		return "", fmt.Errorf("failed to peek item from back: %w", err)
	}

	return ptrToString(itemPtr), nil
}

// IsEmpty checks if the dispenser is empty
func (d *ShmDispenserPattern) IsEmpty() (bool, error) {
	result, _, err := d.isEmpty.Call(d.handle)
	return result != 0, err
}

// IsFull checks if the dispenser is full
func (d *ShmDispenserPattern) IsFull() (bool, error) {
	result, _, err := d.isFull.Call(d.handle)
	return result != 0, err
}


func (d *ShmDispenserPattern) Clear() error {
	_, _, err := d.clear.Call(d.handle)
	return err
}

// Close closes the dispenser and frees resources
func (d *ShmDispenserPattern) Close() error {
	if d.handle != 0 {
		_, _, err := d.close.Call(d.handle)
		if err != nil {
			return fmt.Errorf("failed to close dispenser: %w", err)
		}

		_, _, err = d.destroy.Call(d.handle)
		if err != nil {
			return fmt.Errorf("failed to destroy dispenser: %w", err)
		}

		d.handle = 0
	}

	return nil
}
