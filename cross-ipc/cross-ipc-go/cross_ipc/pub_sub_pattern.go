// pub_sub_pattern.go
package cross_ipc

import (
	"fmt"
	"sync"
	"syscall"
	"unsafe"
)


type MessageHandler func(topic, message string)


type PubSubPattern struct {
	handle    uintptr
	create    *syscall.Proc
	setup     *syscall.Proc
	publish   *syscall.Proc
	subscribe *syscall.Proc
	close     *syscall.Proc
	destroy   *syscall.Proc
	callbacks map[string]MessageHandler
	mu        sync.Mutex 
}

// NewPubSubPattern creates a new publish-subscribe pattern
func NewPubSubPattern(name string, size int, verbose bool) (*PubSubPattern, error) {
	dll, err := loadDLL()
	if err != nil {
		return nil, err
	}

	
	createProc, err := dll.FindProc("PubSubPattern_create")
	if err != nil {
		return nil, fmt.Errorf("failed to find PubSubPattern_create: %w", err)
	}

	setupProc, err := dll.FindProc("PubSubPattern_setup_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find PubSubPattern_setup_api: %w", err)
	}

	publishProc, err := dll.FindProc("PubSubPattern_publish_string_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find PubSubPattern_publish_string_api: %w", err)
	}

	subscribeProc, err := dll.FindProc("PubSubPattern_subscribe_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find PubSubPattern_subscribe_api: %w", err)
	}

	closeProc, err := dll.FindProc("PubSubPattern_close_api")
	if err != nil {
		return nil, fmt.Errorf("failed to find PubSubPattern_close_api: %w", err)
	}

	destroyProc, err := dll.FindProc("PubSubPattern_destroy")
	if err != nil {
		return nil, fmt.Errorf("failed to find PubSubPattern_destroy: %w", err)
	}

	
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
		return nil, fmt.Errorf("failed to create PubSubPattern: %w", err)
	}

	return &PubSubPattern{
		handle:    handle,
		create:    createProc,
		setup:     setupProc,
		publish:   publishProc,
		subscribe: subscribeProc,
		close:     closeProc,
		destroy:   destroyProc,
		callbacks: make(map[string]MessageHandler),
	}, nil
}


func (p *PubSubPattern) Setup() (bool, error) {
	result, _, err := p.setup.Call(p.handle)
	return result != 0, err
}


func (p *PubSubPattern) Publish(topic, message string) error {
	topicBytes := stringToBytes(topic)
	messageBytes := stringToBytes(message)

	_, _, err := p.publish.Call(
		p.handle,
		uintptr(unsafe.Pointer(&topicBytes[0])),
		uintptr(unsafe.Pointer(&messageBytes[0])),
	)

	return err
}

// Subscribe subscribes to a topic with a message handler


func (p *PubSubPattern) Subscribe(topic string) error {
	topicBytes := stringToBytes(topic)

	_, _, err := p.subscribe.Call(
		p.handle,
		uintptr(unsafe.Pointer(&topicBytes[0])),
		0, // No callback support in this simplified version
		0,
	)

	return err
}

// RegisterHandler registers a message handler for a topic


func (p *PubSubPattern) RegisterHandler(topic string, handler MessageHandler) {
	p.mu.Lock()
	defer p.mu.Unlock()
	p.callbacks[topic] = handler
}




func (p *PubSubPattern) PollMessages() {
	
	
}

// Close closes the pattern and frees resources
func (p *PubSubPattern) Close() error {
	if p.handle != 0 {
		_, _, err := p.close.Call(p.handle)
		if err != nil {
			return fmt.Errorf("failed to close PubSubPattern: %w", err)
		}

		_, _, err = p.destroy.Call(p.handle)
		if err != nil {
			return fmt.Errorf("failed to destroy PubSubPattern: %w", err)
		}

		p.handle = 0
	}

	return nil
}
