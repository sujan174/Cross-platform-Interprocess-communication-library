// named_pipe.go
package cross_ipc

import (
	"fmt"
	"syscall"
	"unsafe"
)


type NamedPipe struct {
	handle         uintptr
	create         *syscall.Proc
	createPipe     *syscall.Proc
	connectPipe    *syscall.Proc
	sendMessage    *syscall.Proc
	receiveMessage *syscall.Proc
	closePipe      *syscall.Proc
	destroy        *syscall.Proc
}

// NewNamedPipe creates a new named pipe
func NewNamedPipe(pipeName string, verbose bool) (*NamedPipe, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	
	createProc, err := dll.FindProc("NamedPipe_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_create: %w", err)
	}

	createPipeProc, err := dll.FindProc("NamedPipe_create_pipe_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_create_pipe_api: %w", err)
	}

	connectPipeProc, err := dll.FindProc("NamedPipe_connect_pipe_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_connect_pipe_api: %w", err)
	}

	sendMessageProc, err := dll.FindProc("NamedPipe_send_message_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_send_message_api: %w", err)
	}

	receiveMessageProc, err := dll.FindProc("NamedPipe_receive_message_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_receive_message_api: %w", err)
	}

	closePipeProc, err := dll.FindProc("NamedPipe_close_pipe_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_close_pipe_api: %w", err)
	}

	destroyProc, err := dll.FindProc("NamedPipe_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find NamedPipe_destroy: %w", err)
	}

	
	nameBytes := stringToBytes(pipeName)
	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	handle, _, err := createProc.Call(
		uintptr(unsafe.Pointer(&nameBytes[0])),
		uintptr(verboseInt),
	)

	if handle == 0 {
		return nil, fmt.Errorf("failed to create NamedPipe: %w", err)
	}

	return &NamedPipe{
		handle:         handle,
		create:         createProc,
		createPipe:     createPipeProc,
		connectPipe:    connectPipeProc,
		sendMessage:    sendMessageProc,
		receiveMessage: receiveMessageProc,
		closePipe:      closePipeProc,
		destroy:        destroyProc,
	}, nil
}

// CreatePipe creates the named pipe
func (p *NamedPipe) CreatePipe() error {
	_, _, err := p.createPipe.Call(p.handle)
	return err
}


func (p *NamedPipe) ConnectPipe() error {
	_, _, err := p.connectPipe.Call(p.handle)
	return err
}


func (p *NamedPipe) SendMessage(message string) error {
	messageBytes := stringToBytes(message)

	_, _, err := p.sendMessage.Call(
		p.handle,
		uintptr(unsafe.Pointer(&messageBytes[0])),
	)

	return err
}

// ReceiveMessage receives a message from the pipe
func (p *NamedPipe) ReceiveMessage() (string, error) {
	messagePtr, _, err := p.receiveMessage.Call(p.handle)

	if messagePtr == 0 {
		return "", fmt.Errorf("failed to receive message: %w", err)
	}

	return ptrToString(messagePtr), nil
}

// Close closes the pipe and frees resources
func (p *NamedPipe) Close() error {
	if p.handle != 0 {
		_, _, err := p.closePipe.Call(p.handle)
		if err != nil {
			return fmt.Errorf("failed to close pipe: %w", err)
		}

		_, _, err = p.destroy.Call(p.handle)
		if err != nil {
			return fmt.Errorf("failed to destroy pipe: %w", err)
		}

		p.handle = 0
	}

	return nil
}
