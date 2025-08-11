import time
import json
import random
import string
import argparse
import statistics
import sys
import os
import zmq

# Windows-specific error handling to suppress debug assertion dialogs
if sys.platform == "win32":
    try:
        import ctypes
        
        SEM_NOGPFAULTERRORBOX = 0x0002
        ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
        
        ERROR_SUPPRESS_ABORT = 0
        ctypes.windll.kernel32.SetProcessShutdownParameters(0x4FF, ERROR_SUPPRESS_ABORT)
    except Exception:
        pass


DEFAULT_PAYLOAD_SIZE = 10_000  
DEFAULT_NUM_MESSAGES = 100
DEFAULT_ENDPOINT = "tcp://127.0.0.1:5555"

def generate_random_data(size):
    """Generate random string data of specified size in bytes"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size))

def run_sender(endpoint, payload_size, num_messages):
    
    print(f"Starting ZeroMQ sender on: {endpoint}")
    
    # Initialize ZeroMQ context and socket
    context = zmq.Context()
    
    socket = context.socket(zmq.REQ)
    socket.bind(endpoint)
    
    # Artificially limit high water mark to create backpressure
    socket.setsockopt(zmq.SNDHWM, 1)
    # Disable any batching optimizations
    socket.setsockopt(zmq.IMMEDIATE, 1)
    
    socket.setsockopt(zmq.TCP_KEEPALIVE, 1)
    socket.setsockopt(zmq.TCP_KEEPALIVE_IDLE, 1)
    
    # Allow time for receiver to connect
    print("Waiting for receiver to connect...")
    time.sleep(1)
    
    print(f"Sender running. Will send {num_messages} messages.")
    
    try:
        
        for i in range(num_messages):
            # Create message data with extra padding to increase size
            padding = "X" * (payload_size * 2)  
            message_data = {
                "id": i,
                "timestamp": time.time(),
                "payload": generate_random_data(payload_size),
                "padding": padding
            }
            
            
            message_json = json.dumps(message_data)
            print(f"Sending message {i+1}/{num_messages} (size: {len(message_json)} bytes)")
            socket.send_string(message_json)
            
            
            reply = socket.recv_string()
            
            
            time.sleep(0.1)  # 100ms delay between messages
            
            
            if (i + 1) % 10 == 0 or i == 0:
                print(f"Sent {i + 1}/{num_messages} messages.")
        
        print("\nAll messages sent.")
        
    except KeyboardInterrupt:
        print("Shutting down sender...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        socket.close()
        context.term()
        print("Sender closed.")

def run_receiver(endpoint, num_messages):
    
    print(f"Starting ZeroMQ receiver connecting to: {endpoint}")
    print(f"Waiting for {num_messages} messages...")
    
    
    context = zmq.Context()
    # Use REP socket for the slower REQ/REP pattern
    socket = context.socket(zmq.REP)
    socket.connect(endpoint)
    
    
    socket.setsockopt(zmq.RCVHWM, 1)
    
    # Prepare statistics collection
    latencies = []
    message_times = []
    message_sizes = []
    received_count = 0
    
    try:
        # Receive messages and measure performance
        while received_count < num_messages:
            try:
                # Receive message
                message = socket.recv_string()
                
                # Add artificial processing delay
                time.sleep(0.05)  
                
                try:
                    
                    message_data = json.loads(message)
                    
                    # Calculate latency
                    send_time = message_data.get("timestamp", 0)
                    receive_time = time.time()
                    latency = (receive_time - send_time) * 1000  # ms
                    
                    # Collect statistics
                    latencies.append(latency)
                    message_times.append(receive_time)
                    message_sizes.append(len(message))
                    received_count += 1
                    
                    
                    if received_count % 10 == 0 or received_count == 1:
                        print(f"Received {received_count}/{num_messages} messages. Last latency: {latency:.2f}ms")
                    
                except json.JSONDecodeError:
                    print("Received invalid JSON message")
                except Exception as e:
                    print(f"Error processing message: {e}")
                
                # Must send reply in REQ/REP pattern
                socket.send_string("ACK")
                
            except Exception as e:
                print(f"Error receiving message: {e}")
        
        
        if received_count == 0:
            print("No messages received. Cannot generate statistics.")
            return
            
        print("\n--- ZeroMQ Performance Statistics ---")
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
            "type": "zeromq",
            "pattern": "socket",
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
        
        with open("zeromq_results.json", "w") as f:
            json.dump(results, f, indent=2)
        print("Results saved to zeromq_results.json")
        
    except KeyboardInterrupt:
        print("Shutting down receiver...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        socket.close()
        context.term()
        print("Receiver closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ZeroMQ Socket Benchmark")
    parser.add_argument("--mode", choices=["sender", "receiver"], required=True, 
                        help="Run as sender or receiver")
    parser.add_argument("--endpoint", default=DEFAULT_ENDPOINT, 
                        help=f"ZeroMQ endpoint (default: {DEFAULT_ENDPOINT})")
    parser.add_argument("--size", type=int, default=DEFAULT_PAYLOAD_SIZE, 
                        help=f"Payload size in bytes (sender only, default: {DEFAULT_PAYLOAD_SIZE})")
    parser.add_argument("--messages", type=int, default=DEFAULT_NUM_MESSAGES, 
                        help=f"Number of messages (default: {DEFAULT_NUM_MESSAGES})")
    
    args = parser.parse_args()
    
    if args.mode == "sender":
        run_sender(args.endpoint, args.size, args.messages)
    else:
        run_receiver(args.endpoint, args.messages) 