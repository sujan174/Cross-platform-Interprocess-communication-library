import ctypes
import os
from ctypes import c_char_p, c_size_t, c_bool, c_void_p, POINTER, c_ubyte, c_int, c_ulong


_dll_path = os.path.join(os.path.dirname(__file__), "./cross-ipc.dll")
_lib = ctypes.CDLL(_dll_path)


_lib.NamedPipe_create.argtypes = [c_char_p, c_bool]
_lib.NamedPipe_create.restype = c_void_p

_lib.NamedPipe_destroy.argtypes = [c_void_p]
_lib.NamedPipe_destroy.restype = None

_lib.NamedPipe_create_pipe_api.argtypes = [c_void_p]
_lib.NamedPipe_create_pipe_api.restype = None

_lib.NamedPipe_connect_pipe_api.argtypes = [c_void_p]
_lib.NamedPipe_connect_pipe_api.restype = None

_lib.NamedPipe_send_message_api.argtypes = [c_void_p, c_char_p]
_lib.NamedPipe_send_message_api.restype = None

_lib.NamedPipe_receive_message_api.argtypes = [c_void_p]
_lib.NamedPipe_receive_message_api.restype = c_char_p

_lib.NamedPipe_close_pipe_api.argtypes = [c_void_p]
_lib.NamedPipe_close_pipe_api.restype = None


_lib.OrdinaryPipe_create.argtypes = [c_bool]
_lib.OrdinaryPipe_create.restype = c_void_p

_lib.OrdinaryPipe_destroy.argtypes = [c_void_p]
_lib.OrdinaryPipe_destroy.restype = None

_lib.OrdinaryPipe_create_pipe_api.argtypes = [c_void_p]
_lib.OrdinaryPipe_create_pipe_api.restype = None

_lib.OrdinaryPipe_send_message_api.argtypes = [c_void_p, c_char_p]
_lib.OrdinaryPipe_send_message_api.restype = None

_lib.OrdinaryPipe_receive_message_api.argtypes = [c_void_p]
_lib.OrdinaryPipe_receive_message_api.restype = c_char_p

_lib.OrdinaryPipe_close_pipe_api.argtypes = [c_void_p]
_lib.OrdinaryPipe_close_pipe_api.restype = None


_lib.SharedMemory_create.argtypes = [c_char_p, c_size_t, c_bool]
_lib.SharedMemory_create.restype = c_void_p

_lib.SharedMemory_destroy.argtypes = [c_void_p]
_lib.SharedMemory_destroy.restype = None

_lib.SharedMemory_setup_api.argtypes = [c_void_p]
_lib.SharedMemory_setup_api.restype = c_bool

_lib.SharedMemory_write_api.argtypes = [c_void_p, c_char_p]
_lib.SharedMemory_write_api.restype = None

_lib.SharedMemory_write_bytes_api.argtypes = [c_void_p, POINTER(c_ubyte), c_size_t]
_lib.SharedMemory_write_bytes_api.restype = None

_lib.SharedMemory_read_api.argtypes = [c_void_p]
_lib.SharedMemory_read_api.restype = c_char_p

_lib.SharedMemory_read_bytes_api.argtypes = [c_void_p, POINTER(c_size_t)]
_lib.SharedMemory_read_bytes_api.restype = POINTER(c_ubyte)

_lib.SharedMemory_clear_api.argtypes = [c_void_p]
_lib.SharedMemory_clear_api.restype = None

_lib.SharedMemory_close_api.argtypes = [c_void_p]
_lib.SharedMemory_close_api.restype = None

_lib.SharedMemory_unlink_api.argtypes = [c_void_p]
_lib.SharedMemory_unlink_api.restype = None

# New lock-related API functions for SharedMemory
_lib.SharedMemory_lock_for_writing_api.argtypes = [c_void_p, c_ulong]
_lib.SharedMemory_lock_for_writing_api.restype = c_bool

_lib.SharedMemory_unlock_from_writing_api.argtypes = [c_void_p]
_lib.SharedMemory_unlock_from_writing_api.restype = c_bool

# StoreDictPattern API
_lib.StoreDictPattern_create.argtypes = [c_char_p, c_size_t, c_bool]
_lib.StoreDictPattern_create.restype = c_void_p

_lib.StoreDictPattern_destroy.argtypes = [c_void_p]
_lib.StoreDictPattern_destroy.restype = None

_lib.StoreDictPattern_setup_api.argtypes = [c_void_p]
_lib.StoreDictPattern_setup_api.restype = c_bool

_lib.StoreDictPattern_store_string_api.argtypes = [c_void_p, c_char_p, c_char_p]
_lib.StoreDictPattern_store_string_api.restype = None

_lib.StoreDictPattern_retrieve_string_api.argtypes = [c_void_p, c_char_p]
_lib.StoreDictPattern_retrieve_string_api.restype = c_char_p

_lib.StoreDictPattern_close_api.argtypes = [c_void_p]
_lib.StoreDictPattern_close_api.restype = None

_lib.StoreDictPattern_load_api.argtypes = [c_void_p]
_lib.StoreDictPattern_load_api.restype = None


# Define the callback function type
REQUEST_HANDLER_CALLBACK = ctypes.CFUNCTYPE(c_char_p, c_char_p, c_void_p)

_lib.ReqRespPattern_create.argtypes = [c_bool]
_lib.ReqRespPattern_create.restype = c_void_p

_lib.ReqRespPattern_destroy.argtypes = [c_void_p]
_lib.ReqRespPattern_destroy.restype = None

_lib.ReqRespPattern_setup_server_api.argtypes = [c_void_p, c_char_p]
_lib.ReqRespPattern_setup_server_api.restype = c_bool

_lib.ReqRespPattern_setup_client_api.argtypes = [c_void_p, c_char_p]
_lib.ReqRespPattern_setup_client_api.restype = c_bool

_lib.ReqRespPattern_request_api.argtypes = [c_void_p, c_char_p, c_char_p]
_lib.ReqRespPattern_request_api.restype = c_char_p

_lib.ReqRespPattern_respond_api.argtypes = [c_void_p, c_char_p, REQUEST_HANDLER_CALLBACK, c_void_p]
_lib.ReqRespPattern_respond_api.restype = None

_lib.ReqRespPattern_close_api.argtypes = [c_void_p]
_lib.ReqRespPattern_close_api.restype = None


# Define the callback function type
MESSAGE_HANDLER_CALLBACK = ctypes.CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)

_lib.PubSubPattern_create.argtypes = [c_char_p, c_size_t, c_bool]
_lib.PubSubPattern_create.restype = c_void_p

_lib.PubSubPattern_destroy.argtypes = [c_void_p]
_lib.PubSubPattern_destroy.restype = None

_lib.PubSubPattern_setup_api.argtypes = [c_void_p]
_lib.PubSubPattern_setup_api.restype = c_bool

_lib.PubSubPattern_publish_string_api.argtypes = [c_void_p, c_char_p, c_char_p]
_lib.PubSubPattern_publish_string_api.restype = None

_lib.PubSubPattern_subscribe_api.argtypes = [c_void_p, c_char_p, MESSAGE_HANDLER_CALLBACK, c_void_p]
_lib.PubSubPattern_subscribe_api.restype = None

_lib.PubSubPattern_close_api.argtypes = [c_void_p]
_lib.PubSubPattern_close_api.restype = None



class ShmDispenserMode(ctypes.c_int):
    FIFO = 0
    LIFO = 1
    DEQUE = 2

_lib.ShmDispenserPattern_create.argtypes = [c_char_p, c_int, c_bool]
_lib.ShmDispenserPattern_create.restype = c_void_p

_lib.ShmDispenserPattern_destroy.argtypes = [c_void_p]
_lib.ShmDispenserPattern_destroy.restype = None

_lib.ShmDispenserPattern_setup_api.argtypes = [c_void_p, c_size_t, c_size_t]
_lib.ShmDispenserPattern_setup_api.restype = c_bool

_lib.ShmDispenserPattern_add_string_api.argtypes = [c_void_p, c_char_p]
_lib.ShmDispenserPattern_add_string_api.restype = c_bool

_lib.ShmDispenserPattern_add_string_front_api.argtypes = [c_void_p, c_char_p]
_lib.ShmDispenserPattern_add_string_front_api.restype = c_bool

_lib.ShmDispenserPattern_dispense_string_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_dispense_string_api.restype = c_char_p

_lib.ShmDispenserPattern_dispense_string_back_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_dispense_string_back_api.restype = c_char_p

_lib.ShmDispenserPattern_peek_string_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_peek_string_api.restype = c_char_p

_lib.ShmDispenserPattern_peek_string_back_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_peek_string_back_api.restype = c_char_p

_lib.ShmDispenserPattern_is_empty_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_is_empty_api.restype = c_bool

_lib.ShmDispenserPattern_is_full_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_is_full_api.restype = c_bool

_lib.ShmDispenserPattern_clear_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_clear_api.restype = None

_lib.ShmDispenserPattern_close_api.argtypes = [c_void_p]
_lib.ShmDispenserPattern_close_api.restype = None

class NamedPipe:
    
    def __init__(self, pipe_name, verbose=False):
        self._handle = _lib.NamedPipe_create(pipe_name.encode('utf-8'), verbose)
        if not self._handle:
            raise RuntimeError("Failed to create NamedPipe")
    
    def create_pipe(self):
        
        _lib.NamedPipe_create_pipe_api(self._handle)
    
    def connect_pipe(self):
        
        _lib.NamedPipe_connect_pipe_api(self._handle)
    
    def send_message(self, message):
        """Send a message through the pipe"""
        _lib.NamedPipe_send_message_api(self._handle, message.encode('utf-8'))
    
    def receive_message(self):
        
        result = _lib.NamedPipe_receive_message_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def close(self):
        
        _lib.NamedPipe_close_pipe_api(self._handle)
    
    def __del__(self):
        """Clean up resources when the object is garbage collected"""
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.NamedPipe_destroy(self._handle)

class OrdinaryPipe:
    """
    An ordinary pipe for inter-process communication.
    
    Args:
        verbose (bool): Whether to print debug information
    """
    def __init__(self, verbose=False):
        self._handle = _lib.OrdinaryPipe_create(verbose)
        if not self._handle:
            raise RuntimeError("Failed to create OrdinaryPipe")
    
    def create_pipe(self):
        """Create the pipe"""
        _lib.OrdinaryPipe_create_pipe_api(self._handle)
    
    def send_message(self, message):
        
        _lib.OrdinaryPipe_send_message_api(self._handle, message.encode('utf-8'))
    
    def receive_message(self):
        
        result = _lib.OrdinaryPipe_receive_message_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def close(self):
        """Close the pipe"""
        _lib.OrdinaryPipe_close_pipe_api(self._handle)
    
    def __del__(self):
        
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.OrdinaryPipe_destroy(self._handle)

class SharedMemory:
    
    def __init__(self, id, size, verbose=False):
        self._handle = _lib.SharedMemory_create(id.encode('utf-8'), size, verbose)
        if not self._handle:
            raise RuntimeError("Failed to create SharedMemory")
    
    def setup(self):
        
        return _lib.SharedMemory_setup_api(self._handle)
    
    def write(self, data):
        """Write a string to shared memory"""
        _lib.SharedMemory_write_api(self._handle, data.encode('utf-8'))
    
    def write_bytes(self, data):
        """Write bytes to shared memory"""
        data_array = (c_ubyte * len(data))(*data)
        _lib.SharedMemory_write_bytes_api(self._handle, data_array, len(data))
    
    def read(self):
        
        result = _lib.SharedMemory_read_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def read_bytes(self):
        
        size = c_size_t()
        result = _lib.SharedMemory_read_bytes_api(self._handle, ctypes.byref(size))
        if result and size.value > 0:
            
            return bytes(result[:size.value])
        return None
    
    def clear(self):
        """Clear the shared memory"""
        _lib.SharedMemory_clear_api(self._handle)
    
    def close(self):
        """Close the shared memory resources"""
        _lib.SharedMemory_close_api(self._handle)
    
    def unlink(self):
        
        _lib.SharedMemory_unlink_api(self._handle)
    
    def lock_for_writing(self, timeout_ms=5000):
        
        return _lib.SharedMemory_lock_for_writing_api(self._handle, timeout_ms)
    
    def unlock_from_writing(self):
        
        return _lib.SharedMemory_unlock_from_writing_api(self._handle)
    
    def write_with_lock(self, data, timeout_ms=5000):
        
        if self.lock_for_writing(timeout_ms):
            try:
                self.write(data)
                return True
            finally:
                self.unlock_from_writing()
        return False
    
    def write_bytes_with_lock(self, data, timeout_ms=5000):
        
        if self.lock_for_writing(timeout_ms):
            try:
                self.write_bytes(data)
                return True
            finally:
                self.unlock_from_writing()
        return False
    
    def __del__(self):
        """Clean up resources when the object is garbage collected"""
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.SharedMemory_destroy(self._handle)

class StoreDictPattern:
    
    def __init__(self, name, size=1024, verbose=False):
        self._handle = _lib.StoreDictPattern_create(
            name.encode('utf-8'), size, verbose)
        if not self._handle:
            raise RuntimeError("Failed to create StoreDictPattern")
    
    def setup(self):
        
        return _lib.StoreDictPattern_setup_api(self._handle)
    
    def store(self, key, value):
        """Store a string value with the given key"""
        _lib.StoreDictPattern_store_string_api(
            self._handle, key.encode('utf-8'), value.encode('utf-8'))
    
    def retrieve(self, key):
        """Retrieve a string value for the given key"""
        # First load the latest data from shared memory
        self.load()
        
        result = _lib.StoreDictPattern_retrieve_string_api(
            self._handle, key.encode('utf-8'))
        if result:
            return result.decode('utf-8')
        return None
    
    def load(self):
        
        _lib.StoreDictPattern_load_api(self._handle)
    
    def close(self):
        
        _lib.StoreDictPattern_close_api(self._handle)
    
    def __del__(self):
        
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.StoreDictPattern_destroy(self._handle)

class ReqRespPattern:
    
    def __init__(self, verbose=False):
        self._handle = _lib.ReqRespPattern_create(verbose)
        if not self._handle:
            raise RuntimeError("Failed to create ReqRespPattern")
        self._callbacks = {}  # Store references to prevent garbage collection
    
    def setup_server(self, id):
        """Set up as a server for the given ID"""
        return _lib.ReqRespPattern_setup_server_api(self._handle, id.encode('utf-8'))
    
    def setup_client(self, id):
        
        return _lib.ReqRespPattern_setup_client_api(self._handle, id.encode('utf-8'))
    
    def request(self, id, message):
        
        result = _lib.ReqRespPattern_request_api(
            self._handle, id.encode('utf-8'), message.encode('utf-8'))
        if result:
            return result.decode('utf-8')
        return None
    
    def respond(self, id, handler, user_data=None):
        
        # Create a wrapper function that handles string encoding/decoding
        @REQUEST_HANDLER_CALLBACK
        def callback_wrapper(request, user_data_ptr):
            try:
                # Decode the request
                request_str = request.decode('utf-8')
                
                response = handler(request_str, user_data)
                # Return the encoded response
                if response:
                    return response.encode('utf-8')
                return None
            except Exception as e:
                print(f"Error in request handler: {e}")
                return None
        
        
        self._callbacks[id] = callback_wrapper
        
        # Register the callback with the C library
        _lib.ReqRespPattern_respond_api(
            self._handle, id.encode('utf-8'), callback_wrapper, None)
    
    def close(self):
        """Close all connections and clean up resources"""
        _lib.ReqRespPattern_close_api(self._handle)
    
    def __del__(self):
        """Clean up resources when the object is garbage collected"""
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.ReqRespPattern_destroy(self._handle)

class PubSubPattern:
    """
    A publish-subscribe pattern for inter-process communication.
    
    Args:
        name (str): Unique identifier for this pub-sub system
        size (int): Size of the shared memory in bytes
        verbose (bool): Whether to print debug information
    """
    def __init__(self, name, size=1024, verbose=False):
        self._handle = _lib.PubSubPattern_create(
            name.encode('utf-8'), size, verbose)
        if not self._handle:
            raise RuntimeError("Failed to create PubSubPattern")
        self._callbacks = {}  
    
    def setup(self):
        """Initialize the shared memory and prepare for use"""
        return _lib.PubSubPattern_setup_api(self._handle)
    
    def publish(self, topic, message):
        """Publish a message to a topic"""
        _lib.PubSubPattern_publish_string_api(
            self._handle, topic.encode('utf-8'), message.encode('utf-8'))
    
    def subscribe(self, topic, handler, user_data=None):
        
        
        @MESSAGE_HANDLER_CALLBACK
        def callback_wrapper(topic, payload, user_data_ptr):
            try:
                
                topic_str = topic.decode('utf-8')
                payload_str = payload.decode('utf-8')
                # Call the user's handler
                handler(topic_str, payload_str, user_data)
            except Exception as e:
                print(f"Error in message handler: {e}")
        
        
        key = f"{topic}_{len(self._callbacks)}"
        self._callbacks[key] = callback_wrapper
        
        # Register the callback with the C library
        _lib.PubSubPattern_subscribe_api(
            self._handle, topic.encode('utf-8'), callback_wrapper, None)
    
    def close(self):
        
        _lib.PubSubPattern_close_api(self._handle)
    
    def __del__(self):
        
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.PubSubPattern_destroy(self._handle)

class ShmDispenserPattern:
    
    
    FIFO = ShmDispenserMode.FIFO
    LIFO = ShmDispenserMode.LIFO
    DEQUE = ShmDispenserMode.DEQUE
    
    def __init__(self, name, mode=ShmDispenserMode.FIFO, verbose=False):
        self._handle = _lib.ShmDispenserPattern_create(
            name.encode('utf-8'), mode, verbose)
        if not self._handle:
            raise RuntimeError("Failed to create ShmDispenserPattern")
    
    def setup(self, capacity, item_size):
        """Initialize the shared memory and prepare for use"""
        return _lib.ShmDispenserPattern_setup_api(self._handle, capacity, item_size)
    
    def add(self, item):
        
        return _lib.ShmDispenserPattern_add_string_api(
            self._handle, item.encode('utf-8'))
    
    def add_front(self, item):
        """Add an item to the front of the dispenser (only in DEQUE mode)"""
        return _lib.ShmDispenserPattern_add_string_front_api(
            self._handle, item.encode('utf-8'))
    
    def dispense(self):
        
        result = _lib.ShmDispenserPattern_dispense_string_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def dispense_back(self):
        
        result = _lib.ShmDispenserPattern_dispense_string_back_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def peek(self):
        
        result = _lib.ShmDispenserPattern_peek_string_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def peek_back(self):
        """Return the item at the back of the dispenser without removing it (only in DEQUE mode)"""
        result = _lib.ShmDispenserPattern_peek_string_back_api(self._handle)
        if result:
            return result.decode('utf-8')
        return None
    
    def is_empty(self):
        """Check if the dispenser is empty"""
        return _lib.ShmDispenserPattern_is_empty_api(self._handle)
    
    def is_full(self):
        
        return _lib.ShmDispenserPattern_is_full_api(self._handle)
    
    def clear(self):
        """Remove all items from the dispenser"""
        _lib.ShmDispenserPattern_clear_api(self._handle)
    
    def close(self):
        
        _lib.ShmDispenserPattern_close_api(self._handle)
    
    def __del__(self):
        
        if hasattr(self, '_handle') and self._handle:
            self.close()
            _lib.ShmDispenserPattern_destroy(self._handle)