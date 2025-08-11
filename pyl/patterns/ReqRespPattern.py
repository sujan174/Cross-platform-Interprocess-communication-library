import threading
from multiprocessing import Lock
from .transport.NamedPipe import NamedPipe
import json
import time

class ReqRespPattern:
    def __init__(self):
        """Initialize the request-response pattern."""
        self.server_pipes = {}  # id -> NamedPipe instance
        self.client_pipes = {}  # id -> NamedPipe instance
        self.handlers = {}      # id -> handler function
        self.running = False
        self.listener_threads = {}  # id -> thread
        self.locks = {}        # id -> multiprocessing.Lock

    def setup_server(self, id):
        """Set up the server for handling requests."""
        pipe_name = fr"\\.\pipe\reqresp_{id}"
        self.locks[id] = Lock()
        
        # Create named pipe server
        pipe = NamedPipe(pipe_name)
        pipe.create_pipe()
        self.server_pipes[id] = pipe
        self.running = True
        
        # Start listener thread
        thread = threading.Thread(target=self._listen_for_requests, args=(id,))
        thread.daemon = True
        thread.start()
        self.listener_threads[id] = thread

    def setup_client(self, id):
        """Set up the client for sending requests."""
        pipe_name = fr"\\.\pipe\reqresp_{id}"
        self.locks[id] = Lock()
        
        # Connect to server pipe
        pipe = NamedPipe(pipe_name)
        pipe.connect()
        self.client_pipes[id] = pipe

    def _listen_for_requests(self, id):
        """Listen for incoming requests on a server pipe."""
        while self.running:
            try:
                if not self.running:
                    break
                
                # Wait for client connection
                self.server_pipes[id].connect()
                
                while self.running:
                    try:
                        # Read request
                        request = self.server_pipes[id].receive()
                        if request:
                            # Process request through handler
                            if id in self.handlers:
                                try:
                                    response = self.handlers[id](json.loads(request))
                                    # Send response
                                    response_data = json.dumps(response)
                                    self.server_pipes[id].send(response_data)
                                except Exception as e:
                                    error_response = json.dumps({"error": str(e)})
                                    self.server_pipes[id].send(error_response)
                        else:
                            break  # Client disconnected
                            
                    except Exception as e:
                        if not self.running:
                            break
                        print(f"Error handling request on {id}: {e}")
                        break
                            
            except Exception as e:
                if not self.running:
                    break
                print(f"Server error on {id}: {e}")
                time.sleep(1)  # Avoid tight loop on error

    def request(self, id, message):
        """Send a request and wait for response."""
        if id not in self.locks:
            self.locks[id] = Lock()
            
        try:
            self.locks[id].acquire()
            # Send request
            request_data = json.dumps(message)
            self.client_pipes[id].send(request_data)
            
            # Read response
            response = self.client_pipes[id].receive()
            if response:
                return json.loads(response)
                
        finally:
            if id in self.locks:
                self.locks[id].release()
        
        return None

    def respond(self, id, handler):
        """Set up a handler to respond to requests."""
        self.handlers[id] = handler

    def close(self):
        """Close the request-response system."""
        self.running = False
        
        # Wait for listener threads to finish
        for id, thread in self.listener_threads.items():
            if thread.is_alive():
                thread.join(timeout=1.0)
        
        # Close all pipes
        for pipe in self.server_pipes.values():
            pipe.close()
        for pipe in self.client_pipes.values():
            pipe.close()
            
        # Clear collections
        self.server_pipes.clear()
        self.client_pipes.clear()
        self.handlers.clear()
        self.locks.clear()


