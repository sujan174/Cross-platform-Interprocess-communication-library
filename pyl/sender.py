from patterns.ReqRespPattern import ReqRespPattern
import time
import signal
import sys

def signal_handler(sig, frame):
    print("\nShutting down server...")
    server.close()
    sys.exit(0)

def request_handler(request):
    print(f"Received request: {request}")
    return {"status": "success", "echo": request}

# Set up signal handler
signal.signal(signal.SIGINT, signal_handler)

# Create and setup server
server = ReqRespPattern()
server.setup_server("test_server")
server.respond("test_server", request_handler)

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("\nShutting down server...")
    server.close()
    sys.exit(0)