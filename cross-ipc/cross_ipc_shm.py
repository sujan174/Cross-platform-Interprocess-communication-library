import time
import json
import random
import string
import argparse
import statistics
import sys
import os
from cross_ipc import SharedMemory

# Windows-specific error handling to suppress debug assertion dialogs
if sys.platform == "win32":
    try:
        import ctypes
        # Disable Windows Error Reporting dialog for Python process
        SEM_NOGPFAULTERRORBOX = 0x0002
        ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
        
        ERROR_SUPPRESS_ABORT = 0
        ctypes.windll.kernel32.SetProcessShutdownParameters(0x4FF, ERROR_SUPPRESS_ABORT)
    except Exception:
        pass


DEFAULT_PAYLOAD_SIZE = 10_000  
DEFAULT_NUM_MESSAGES = 100
DEFAULT_NAME = "benchmark_shm"
DEFAULT_SHM_SIZE = 20 * 1024 * 1024  # 20MB

def generate_random_data(size):
    """Generate random string data of specified size in bytes"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size))

def run_sender(name, payload_size, num_messages):
    
    print(f"Starting Cross-IPC SharedMemory sender with name: {name}")
    
    
    shm = SharedMemory(name, DEFAULT_SHM_SIZE, verbose=True)
    
    print("Setting up shared memory...")
    success = shm.setup()
    
    if not success:
        print("Failed to set up shared memory.")
        return
    
    print(f"Sender running. Will send {num_messages} messages.")
    
    try:
        
        for i in range(num_messages):
            
            message_data = {
                "id": i,
                "timestamp": time.time(),
                "payload": generate_random_data(payload_size)
            }
            
            # Convert to JSON and write to shared memory
            message_json = json.dumps(message_data)
            print(f"Sending message {i+1}/{num_messages} (size: {len(message_json)} bytes)")
            
            
            shm.clear()
            
            
            shm.write(message_json)
            
            
            if (i + 1) % 10 == 0 or i == 0:
                print(f"Sent {i + 1}/{num_messages} messages.")
            
            # Delay to allow receiver to process
            time.sleep(0.05)
        
        print("\nAll messages sent.")
        
        
        shm.write(json.dumps({"complete": True}))
        
        
        print("Waiting for receiver to process final messages...")
        time.sleep(2)
        
    except KeyboardInterrupt:
        print("Shutting down sender...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        shm.close()
        print("Sender closed.")

def run_receiver(name, num_messages):
    """Run the SharedMemory receiver and measure performance"""
    print(f"Starting Cross-IPC SharedMemory receiver with name: {name}")
    print(f"Waiting for {num_messages} messages...")
    
    
    shm = SharedMemory(name, DEFAULT_SHM_SIZE, verbose=True)
    
    print("Setting up shared memory...")
    success = shm.setup()
    
    if not success:
        print("Failed to set up shared memory.")
        return
    
    # Prepare statistics collection
    latencies = []
    message_times = []
    message_sizes = []
    received_count = 0
    
    try:
        
        start_time = time.time()
        timeout = 60  # 60 seconds timeout
        last_data = None
        
        while received_count < num_messages:
            # Check for timeout
            if time.time() - start_time > timeout:
                print(f"Timeout after {timeout} seconds. Received {received_count}/{num_messages} messages.")
                break
            
            
            data = shm.read()
            
            
            if not data or data == last_data:
                time.sleep(0.01)
                continue
                
            last_data = data
            
            try:
                # Parse message
                message_data = json.loads(data)
                
                # Check if this is the completion message
                if message_data.get("complete", False):
                    print("Received completion message.")
                    break
                
                
                send_time = message_data.get("timestamp", 0)
                receive_time = time.time()
                latency = (receive_time - send_time) * 1000  
                
                # Collect statistics
                latencies.append(latency)
                message_times.append(receive_time)
                message_sizes.append(len(data))
                received_count += 1
                
                
                if received_count % 10 == 0 or received_count == 1:
                    print(f"Received {received_count}/{num_messages} messages. Last latency: {latency:.2f}ms")
                
            except json.JSONDecodeError:
                print("Received invalid JSON message")
            except Exception as e:
                print(f"Error processing message: {e}")
            
            
            time.sleep(0.01)
        
        # Calculate and print statistics
        if received_count == 0:
            print("No messages received. Cannot generate statistics.")
            return
            
        print("\n--- Cross-IPC SharedMemory Performance Statistics ---")
        print(f"Total messages received: {received_count}")
        
        if message_sizes:
            print(f"Average message size: {sum(message_sizes) / len(message_sizes):.2f} bytes")
        
        print(f"Average latency: {statistics.mean(latencies):.2f} ms")
        print(f"Median latency: {statistics.median(latencies):.2f} ms")
        print(f"Min latency: {min(latencies):.2f} ms")
        print(f"Max latency: {max(latencies):.2f} ms")
        
        if len(latencies) > 1:
            print(f"Latency std dev: {statistics.stdev(latencies):.2f} ms")
        
        
        if len(message_times) >= 2:
            total_time = message_times[-1] - message_times[0]
            messages_per_second = (len(message_times) - 1) / total_time
            print(f"Messages per second: {messages_per_second:.2f}")
            print(f"Total time: {total_time:.2f} seconds")
        
        # Save results to file
        results = {
            "type": "cross_ipc",
            "pattern": "sharedmemory",
            "num_messages": num_messages,
            "messages_received": received_count,
            "latencies": latencies,
            "mean_latency": statistics.mean(latencies),
            "median_latency": statistics.median(latencies),
            "min_latency": min(latencies),
            "max_latency": max(latencies),
            "stdev_latency": statistics.stdev(latencies) if len(latencies) > 1 else 0,
            "messages_per_second": messages_per_second if len(message_times) >= 2 else None,
            "total_time": total_time if len(message_times) >= 2 else None,
        }
        
        with open("cross_ipc_results.json", "w") as f:
            json.dump(results, f, indent=2)
        print("Results saved to cross_ipc_results.json")
        
    except KeyboardInterrupt:
        print("Shutting down receiver...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        shm.close()
        print("Receiver closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Cross-IPC SharedMemory Benchmark")
    parser.add_argument("--mode", choices=["sender", "receiver"], required=True, 
                        help="Run as sender or receiver")
    parser.add_argument("--name", default=DEFAULT_NAME, 
                        help=f"Unique name for the shared memory (default: {DEFAULT_NAME})")
    parser.add_argument("--size", type=int, default=DEFAULT_PAYLOAD_SIZE, 
                        help=f"Payload size in bytes (sender only, default: {DEFAULT_PAYLOAD_SIZE})")
    parser.add_argument("--messages", type=int, default=DEFAULT_NUM_MESSAGES, 
                        help=f"Number of messages (default: {DEFAULT_NUM_MESSAGES})")
    
    args = parser.parse_args()
    
    if args.mode == "sender":
        run_sender(args.name, args.size, args.messages)
    else:
        run_receiver(args.name, args.messages) 