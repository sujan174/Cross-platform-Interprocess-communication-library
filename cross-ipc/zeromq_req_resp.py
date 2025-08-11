import time
import json
import random
import string
import argparse
import statistics
import sys
import os
import zmq


if sys.platform == "win32":
    try:
        import ctypes
        
        SEM_NOGPFAULTERRORBOX = 0x0002
        ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
        
        ERROR_SUPPRESS_ABORT = 0
        ctypes.windll.kernel32.SetProcessShutdownParameters(0x4FF, ERROR_SUPPRESS_ABORT)
    except Exception:
        pass


DEFAULT_PAYLOAD_SIZE = 100_000  
DEFAULT_NUM_REQUESTS = 1000
DEFAULT_ENDPOINT = "tcp://127.0.0.1:5555"

def generate_random_data(size):
    """Generate random string data of specified size in bytes"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size))

def run_server(endpoint, payload_size):
    
    print(f"Starting ZeroMQ server on: {endpoint}")
    
    # Initialize ZeroMQ context and socket
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind(endpoint)
    
    print(f"Server running. Press Ctrl+C to stop.")
    
    try:
        while True:
            # Wait for request
            request = socket.recv_string()
            
            # Parse the request
            req_data = json.loads(request)
            
            
            if "echo" in req_data:
                
                socket.send_string(request)
            else:
                
                response_data = {
                    "id": req_data["id"],
                    "timestamp": time.time(),
                    "payload": generate_random_data(payload_size)
                }
                socket.send_string(json.dumps(response_data))
    
    except KeyboardInterrupt:
        print("Shutting down server...")
    finally:
        socket.close()
        context.term()
        print("Server closed.")

def run_client(endpoint, payload_size, num_requests):
    
    print(f"Starting ZeroMQ client connecting to: {endpoint}")
    print(f"Sending {num_requests} requests with payload size: {payload_size} bytes")
    
    # Initialize ZeroMQ context and socket
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect(endpoint)
    
    # Set a timeout to prevent hanging
    socket.setsockopt(zmq.RCVTIMEO, 5000)  
    
    
    latencies = []
    request_times = []
    response_sizes = []
    
    try:
        
        for i in range(num_requests):
            
            request_data = {
                "id": i,
                "timestamp": time.time(),
                "echo": False  
            }
            
            try:
                # Measure request-response time
                start_time = time.time()
                socket.send_string(json.dumps(request_data))
                response = socket.recv_string()
                end_time = time.time()
                
                # Calculate metrics
                latency = (end_time - start_time) * 1000  
                latencies.append(latency)
                request_times.append(end_time)
                
                # Parse response
                if response:
                    response_sizes.append(len(response))
                else:
                    print(f"Warning: Empty response received for request {i}")
                
                
                if (i + 1) % 100 == 0 or i == 0:
                    print(f"Completed {i + 1}/{num_requests} requests. Last latency: {latency:.2f}ms")
                
                
                time.sleep(0.001)
                
            except zmq.Again:
                print(f"Timeout on request {i}")
                # Reset socket on timeout
                socket.close()
                socket = context.socket(zmq.REQ)
                socket.connect(endpoint)
                socket.setsockopt(zmq.RCVTIMEO, 5000)
                time.sleep(0.1)
            except Exception as e:
                print(f"Error on request {i}: {e}")
                time.sleep(0.1)
        
        # Skip statistics if no successful requests
        if not latencies:
            print("No successful requests completed. Cannot generate statistics.")
            return
            
        
        print("\n--- ZeroMQ Performance Statistics ---")
        print(f"Total successful requests: {len(latencies)} of {num_requests}")
        
        if response_sizes:
            print(f"Average response size: {sum(response_sizes) / len(response_sizes):.2f} bytes")
        
        print(f"Average latency: {statistics.mean(latencies):.2f} ms")
        print(f"Median latency: {statistics.median(latencies):.2f} ms")
        print(f"Min latency: {min(latencies):.2f} ms")
        print(f"Max latency: {max(latencies):.2f} ms")
        
        if len(latencies) > 1:
            print(f"Latency std dev: {statistics.stdev(latencies):.2f} ms")
        
        # Calculate requests per second
        if len(request_times) >= 2:
            total_time = request_times[-1] - request_times[0]
            requests_per_second = (len(latencies) - 1) / total_time
            print(f"Requests per second: {requests_per_second:.2f}")
            print(f"Total time: {total_time:.2f} seconds")
        
        # Save results to file
        results = {
            "type": "zeromq",
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
        
        with open("zeromq_results.json", "w") as f:
            json.dump(results, f, indent=2)
        print("Results saved to zeromq_results.json")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        socket.close()
        context.term()
        print("Client closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ZeroMQ Request-Response Benchmark")
    parser.add_argument("--mode", choices=["server", "client"], required=True, 
                        help="Run as server or client")
    parser.add_argument("--endpoint", default=DEFAULT_ENDPOINT, 
                        help=f"ZeroMQ endpoint (default: {DEFAULT_ENDPOINT})")
    parser.add_argument("--size", type=int, default=DEFAULT_PAYLOAD_SIZE, 
                        help=f"Payload size in bytes (default: {DEFAULT_PAYLOAD_SIZE})")
    parser.add_argument("--requests", type=int, default=DEFAULT_NUM_REQUESTS, 
                        help=f"Number of requests to send (client only, default: {DEFAULT_NUM_REQUESTS})")
    
    args = parser.parse_args()
    
    if args.mode == "server":
        run_server(args.endpoint, args.size)
    else:
        run_client(args.endpoint, args.size, args.requests) 