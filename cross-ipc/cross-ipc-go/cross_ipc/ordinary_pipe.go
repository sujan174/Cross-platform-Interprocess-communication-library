
package cross_ipc

import (
	"fmt"
	"syscall"
	"unsafe"
)


type OrdinaryPipe struct {
	handle         uintptr
	create         *syscall.Proc
	createPipe     *syscall.Proc
	sendMessage    *syscall.Proc
	receiveMessage *syscall.Proc
	closePipe      *syscall.Proc
	destroy        *syscall.Proc
}


func NewOrdinaryPipe(verbose bool) (*OrdinaryPipe, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	
	createProc, err := dll.FindProc("OrdinaryPipe_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find OrdinaryPipe_create: %w", err)
	}

	createPipeProc, err := dll.FindProc("OrdinaryPipe_create_pipe_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find OrdinaryPipe_create_pipe_api: %w", err)
	}

	sendMessageProc, err := dll.FindProc("OrdinaryPipe_send_message_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find OrdinaryPipe_send_message_api: %w", err)
	}

	receiveMessageProc, err := dll.FindProc("OrdinaryPipe_receive_message_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find OrdinaryPipe_receive_message_api: %w", err)
	}

	closePipeProc, err := dll.FindProc("OrdinaryPipe_close_pipe_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find OrdinaryPipe_close_pipe_api: %w", err)
	}

	destroyProc, err := dll.FindProc("OrdinaryPipe_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find OrdinaryPipe_destroy: %w", err)
	}

	
	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	handle, _, err := createProc.Call(uintptr(verboseInt))

	if handle == 0 {
		return nil, fmt.Errorf("failed to create OrdinaryPipe: %w", err)
	}

	return &OrdinaryPipe{
		handle:         handle,
		create:         createProc,
		createPipe:     createPipeProc,
		sendMessage:    sendMessageProc,
		receiveMessage: receiveMessageProc,
		closePipe:      closePipeProc,
		destroy:        destroyProc,
	}, nil
}


func (p *OrdinaryPipe) CreatePipe() error {
	_, _, err := p.createPipe.Call(p.handle)
	return err
}

// SendMessage sends a message through the pipe
func (p *OrdinaryPipe) SendMessage(message string) error {
	messageBytes := stringToBytes(message)

	_, _, err := p.sendMessage.Call(
		p.handle,
		uintptr(unsafe.Pointer(&messageBytes[0])),
	)

	return err
}

// ReceiveMessage receives a message from the pipe
func (p *OrdinaryPipe) ReceiveMessage() (string, error) {
	messagePtr, _, err := p.receiveMessage.Call(p.handle)

	if messagePtr == 0 {
		return "", fmt.Errorf("failed to receive message: %w", err)
	}

	return ptrToString(messagePtr), nil
}

// Close closes the pipe and frees resources
func (p *OrdinaryPipe) Close() error {
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
