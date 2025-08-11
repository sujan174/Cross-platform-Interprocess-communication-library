
package cross_ipc

import (
	"fmt"
	"sync"
	"syscall"
	"unsafe"
)

// RequestHandler is the function type for handling requests
type RequestHandler func(request string) string


type ReqRespPattern struct {
	handle      uintptr
	create      *syscall.Proc
	setupServer *syscall.Proc
	setupClient *syscall.Proc
	request     *syscall.Proc
	respond     *syscall.Proc
	close       *syscall.Proc
	destroy     *syscall.Proc
	callbacks   map[string]RequestHandler
	mu          sync.Mutex // Protects callbacks
}

// NewReqRespPattern creates a new request-response pattern
func NewReqRespPattern(verbose bool) (*ReqRespPattern, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	// Get function pointers
	createProc, err := dll.FindProc("ReqRespPattern_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_create: %w", err)
	}

	setupServerProc, err := dll.FindProc("ReqRespPattern_setup_server_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_setup_server_api: %w", err)
	}

	setupClientProc, err := dll.FindProc("ReqRespPattern_setup_client_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_setup_client_api: %w", err)
	}

	requestProc, err := dll.FindProc("ReqRespPattern_request_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_request_api: %w", err)
	}

	respondProc, err := dll.FindProc("ReqRespPattern_respond_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_respond_api: %w", err)
	}

	closeProc, err := dll.FindProc("ReqRespPattern_close_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_close_api: %w", err)
	}

	destroyProc, err := dll.FindProc("ReqRespPattern_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find ReqRespPattern_destroy: %w", err)
	}

	
	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	handle, _, err := createProc.Call(uintptr(verboseInt))

	if handle == 0 {
		return nil, fmt.Errorf("failed to create ReqRespPattern: %w", err)
	}

	return &ReqRespPattern{
		handle:      handle,
		create:      createProc,
		setupServer: setupServerProc,
		setupClient: setupClientProc,
		request:     requestProc,
		respond:     respondProc,
		close:       closeProc,
		destroy:     destroyProc,
		callbacks:   make(map[string]RequestHandler),
	}, nil
}


func (r *ReqRespPattern) SetupServer(id string) (bool, error) {
	idBytes := stringToBytes(id)

	result, _, err := r.setupServer.Call(
		r.handle,
		uintptr(unsafe.Pointer(&idBytes[0])),
	)

	return result != 0, err
}


func (r *ReqRespPattern) SetupClient(id string) (bool, error) {
	idBytes := stringToBytes(id)

	result, _, err := r.setupClient.Call(
		r.handle,
		uintptr(unsafe.Pointer(&idBytes[0])),
	)

	return result != 0, err
}


func (r *ReqRespPattern) Request(id, message string) (string, error) {
	idBytes := stringToBytes(id)
	messageBytes := stringToBytes(message)

	responsePtr, _, err := r.request.Call(
		r.handle,
		uintptr(unsafe.Pointer(&idBytes[0])),
		uintptr(unsafe.Pointer(&messageBytes[0])),
	)

	if responsePtr == 0 {
		return "", fmt.Errorf("failed to get response: %w", err)
	}

	return ptrToString(responsePtr), nil
}

// Respond registers a handler for responding to requests
// Note: This is a simplified version that doesn't support callbacks from C to Go

func (r *ReqRespPattern) Respond(id string, handler RequestHandler) error {
	r.mu.Lock()
	r.callbacks[id] = handler
	r.mu.Unlock()

	
	
	idBytes := stringToBytes(id)

	_, _, err := r.respond.Call(
		r.handle,
		uintptr(unsafe.Pointer(&idBytes[0])),
		0, // No callback support in this simplified version
		0,
	)

	return err
}

// PollRequests polls for incoming requests


func (r *ReqRespPattern) PollRequests() {
	
	
}


func (r *ReqRespPattern) Close() error {
	if r.handle != 0 {
		_, _, err := r.close.Call(r.handle)
		if err != nil {
			return fmt.Errorf("failed to close ReqRespPattern: %w", err)
		}

		_, _, err = r.destroy.Call(r.handle)
		if err != nil {
			return fmt.Errorf("failed to destroy ReqRespPattern: %w", err)
		}

		r.handle = 0
	}

	return nil
}
