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

# Constants
DEFAULT_PAYLOAD_SIZE = 100_000  # 100KB
DEFAULT_NUM_MESSAGES = 1000
DEFAULT_ENDPOINT = "tcp://127.0.0.1:5556"
DEFAULT_TOPIC = "benchmark"

def generate_random_data(size):
    """Generate random string data of specified size in bytes"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size))

def run_publisher(endpoint, topic, payload_size, num_messages):
    """Run the ZeroMQ PUB publisher"""
    print(f"Starting ZeroMQ publisher on: {endpoint}")
    print(f"Topic: {topic}")
    
    
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind(endpoint)
    
    
    print("Waiting for subscribers to connect...")
    time.sleep(1)
    
    print(f"Publisher running. Will send {num_messages} messages.")
    
    try:
        
        for i in range(num_messages):
            
            message_data = {
                "id": i,
                "timestamp": time.time(),
                "payload": generate_random_data(payload_size)
            }
            
            # Publish the message
            socket.send_string(f"{topic} {json.dumps(message_data)}")
            
            
            if (i + 1) % 100 == 0 or i == 0:
                print(f"Published {i + 1}/{num_messages} messages.")
            
            # Small delay to prevent overwhelming the system
            time.sleep(0.001)
        
        print("\nAll messages published.")
        
        # Allow time for last messages to be delivered
        time.sleep(0.5)
        
    except KeyboardInterrupt:
        print("Shutting down publisher...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        socket.close()
        context.term()
        print("Publisher closed.")

def run_subscriber(endpoint, topic, num_messages):
    """Run the ZeroMQ SUB subscriber and measure performance"""
    print(f"Starting ZeroMQ subscriber connecting to: {endpoint}")
    print(f"Topic: {topic}")
    print(f"Waiting for {num_messages} messages...")
    
    
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(endpoint)
    socket.setsockopt_string(zmq.SUBSCRIBE, topic)
    
    
    socket.setsockopt(zmq.RCVTIMEO, 5000)  
    
    
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
                
                
                _, payload = message.split(' ', 1)
                message_data = json.loads(payload)
                
                
                send_time = message_data.get("timestamp", 0)
                receive_time = time.time()
                latency = (receive_time - send_time) * 1000  
                
                
                latencies.append(latency)
                message_times.append(receive_time)
                message_sizes.append(len(payload))
                received_count += 1
                
                
                if received_count % 100 == 0 or received_count == 1:
                    print(f"Received {received_count}/{num_messages} messages. Last latency: {latency:.2f}ms")
                
            except zmq.Again:
                print("No message received within timeout period")
            except Exception as e:
                print(f"Error receiving message: {e}")
        
        # Calculate and print statistics
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
        
        # Calculate messages per second
        if len(message_times) >= 2:
            total_time = message_times[-1] - message_times[0]
            messages_per_second = (len(message_times) - 1) / total_time
            print(f"Messages per second: {messages_per_second:.2f}")
            print(f"Total time: {total_time:.2f} seconds")
        
        
        results = {
            "type": "zeromq",
            "pattern": "pubsub",
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
        print("Shutting down subscriber...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        socket.close()
        context.term()
        print("Subscriber closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ZeroMQ Pub-Sub Benchmark")
    parser.add_argument("--mode", choices=["publisher", "subscriber"], required=True, 
                        help="Run as publisher or subscriber")
    parser.add_argument("--endpoint", default=DEFAULT_ENDPOINT, 
                        help=f"ZeroMQ endpoint (default: {DEFAULT_ENDPOINT})")
    parser.add_argument("--topic", default=DEFAULT_TOPIC, 
                        help=f"Topic name (default: {DEFAULT_TOPIC})")
    parser.add_argument("--size", type=int, default=DEFAULT_PAYLOAD_SIZE, 
                        help=f"Payload size in bytes (publisher only, default: {DEFAULT_PAYLOAD_SIZE})")
    parser.add_argument("--messages", type=int, default=DEFAULT_NUM_MESSAGES, 
                        help=f"Number of messages (default: {DEFAULT_NUM_MESSAGES})")
    
    args = parser.parse_args()
    
    if args.mode == "publisher":
        run_publisher(args.endpoint, args.topic, args.size, args.messages)
    else:
        run_subscriber(args.endpoint, args.topic, args.messages) 