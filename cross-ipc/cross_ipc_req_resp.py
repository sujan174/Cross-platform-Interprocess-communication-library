import time
import json
import random
import string
import argparse
import statistics
import sys
import os
from cross_ipc import ReqRespPattern

# Windows-specific error handling to suppress debug assertion dialogs
if sys.platform == "win32":
    try:
        import ctypes
        # Disable Windows Error Reporting dialog for Python process
        SEM_NOGPFAULTERRORBOX = 0x0002
        ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
        # Disable the "abort/retry/ignore" message boxes
        ERROR_SUPPRESS_ABORT = 0
        ctypes.windll.kernel32.SetProcessShutdownParameters(0x4FF, ERROR_SUPPRESS_ABORT)
    except Exception:
        pass


DEFAULT_PAYLOAD_SIZE = 100_000  
DEFAULT_NUM_REQUESTS = 1000
DEFAULT_ID = "benchmark_req_resp"

def generate_random_data(size):
    
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size))

def run_server(id, payload_size):
    
    print(f"Starting Cross-IPC server with ID: {id}")
    
    
    server = ReqRespPattern(verbose=False)
    server.setup_server(id)
    
    # Define the handler function
    def request_handler(request, user_data):
        
        req_data = json.loads(request)
        
        # Create a response with the requested size
        if "echo" in req_data:
            
            return request
        else:
            
            response_data = {
                "id": req_data["id"],
                "timestamp": time.time(),
                "payload": generate_random_data(payload_size)
            }
            return json.dumps(response_data)
    
    
    server.respond(id, request_handler)
    
    print(f"Server running. Press Ctrl+C to stop.")
    
    try:
        # Keep the server running
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("Shutting down server...")
    finally:
        server.close()
        print("Server closed.")

def run_client(id, payload_size, num_requests):
    
    print(f"Starting Cross-IPC client with ID: {id}")
    print(f"Sending {num_requests} requests with payload size: {payload_size} bytes")
    
    
    client = ReqRespPattern(verbose=False)
    success = client.setup_client(id)
    
    if not success:
        print("Failed to set up client. Make sure the server is running first.")
        return
    
    
    latencies = []
    request_times = []
    response_sizes = []
    
    try:
        # Send requests and measure performance
        for i in range(num_requests):
            # Create request data
            request_data = {
                "id": i,
                "timestamp": time.time(),
                "echo": False  
            }
            
            
            start_time = time.time()
            try:
                response = client.request(id, json.dumps(request_data))
                end_time = time.time()
                
                # Calculate metrics
                latency = (end_time - start_time) * 1000  # ms
                latencies.append(latency)
                request_times.append(end_time)
                
                
                if response:
                    response_sizes.append(len(response))
                else:
                    print(f"Warning: Empty response received for request {i}")
                
                
                if (i + 1) % 100 == 0 or i == 0:
                    print(f"Completed {i + 1}/{num_requests} requests. Last latency: {latency:.2f}ms")
                
                
                time.sleep(0.001)
                
            except Exception as e:
                print(f"Error on request {i}: {e}")
                
                time.sleep(0.1)
        
        # Skip statistics if no successful requests
        if not latencies:
            print("No successful requests completed. Cannot generate statistics.")
            return
            
        
        print("\n--- Cross-IPC Performance Statistics ---")
        print(f"Total successful requests: {len(latencies)} of {num_requests}")
        
        if response_sizes:
            print(f"Average response size: {sum(response_sizes) / len(response_sizes):.2f} bytes")
        
        print(f"Average latency: {statistics.mean(latencies):.2f} ms")
        print(f"Median latency: {statistics.median(latencies):.2f} ms")
        print(f"Min latency: {min(latencies):.2f} ms")
        print(f"Max latency: {max(latencies):.2f} ms")
        
        if len(latencies) > 1:
            print(f"Latency std dev: {statistics.stdev(latencies):.2f} ms")
        
        
        if len(request_times) >= 2:
            total_time = request_times[-1] - request_times[0]
            requests_per_second = (len(latencies) - 1) / total_time
            print(f"Requests per second: {requests_per_second:.2f}")
            print(f"Total time: {total_time:.2f} seconds")
        
        # Save results to file
        results = {
            "type": "cross_ipc",
            "payload_size": payload_size,
            "num_requests": num_requests,
            "successful_requests": len(latencies),
            "latencies": latencies,
            "mean_latency": statistics.mean(latencies),
            "median_latency": statistics.median(latencies),
            "min_latency": min(latencies),
            "max_latency": max(latencies),
            "stdev_latency": statistics.stdev(latencies) if len(latencies) > 1 else 0,
            "requests_per_second": requests_per_second if len(request_times) >= 2 else None,
            "total_time": total_time if len(request_times) >= 2 else None,
        }
        
        with open("cross_ipc_results.json", "w") as f:
            json.dump(results, f, indent=2)
        print("Results saved to cross_ipc_results.json")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client.close()
        print("Client closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Cross-IPC Request-Response Benchmark")
    parser.add_argument("--mode", choices=["server", "client"], required=True, 
                        help="Run as server or client")
    parser.add_argument("--id", default=DEFAULT_ID, 
                        help=f"Unique ID for the req-resp pattern (default: {DEFAULT_ID})")
    parser.add_argument("--size", type=int, default=DEFAULT_PAYLOAD_SIZE, 
                        help=f"Payload size in bytes (default: {DEFAULT_PAYLOAD_SIZE})")
    parser.add_argument("--requests", type=int, default=DEFAULT_NUM_REQUESTS, 
                        help=f"Number of requests to send (client only, default: {DEFAULT_NUM_REQUESTS})")
    
    args = parser.parse_args()
    
    if args.mode == "server":
        run_server(args.id, args.size)
    else:
        run_client(args.id, args.size, args.requests) 