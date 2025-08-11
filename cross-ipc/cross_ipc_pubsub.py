import time
import json
import random
import string
import argparse
import statistics
import sys
import os
from cross_ipc import PubSubPattern


if sys.platform == "win32":
    try:
        import ctypes
        
        SEM_NOGPFAULTERRORBOX = 0x0002
        ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
        # Disable the "abort/retry/ignore" message boxes
        ERROR_SUPPRESS_ABORT = 0
        ctypes.windll.kernel32.SetProcessShutdownParameters(0x4FF, ERROR_SUPPRESS_ABORT)
    except Exception:
        pass

# Constants
DEFAULT_PAYLOAD_SIZE = 10_000  # 10KB - reduced from 100KB
DEFAULT_NUM_MESSAGES = 100     # reduced from 1000
DEFAULT_TOPIC = "benchmark_topic"
DEFAULT_NAME = "benchmark_pubsub"
DEFAULT_SHM_SIZE = 10 * 1024 * 1024  

def generate_random_data(size):
    """Generate random string data of specified size in bytes"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size))

def run_publisher(name, topic, payload_size, num_messages):
    
    print(f"Starting Cross-IPC publisher with name: {name}, topic: {topic}")
    
    
    publisher = PubSubPattern(name, DEFAULT_SHM_SIZE, verbose=True)  # Set verbose to True for debugging
    
    print("Setting up publisher...")
    success = publisher.setup()
    
    if not success:
        print("Failed to set up publisher.")
        return
    
    print(f"Publisher running. Will send {num_messages} messages.")
    
    try:
        # Send messages
        for i in range(num_messages):
            # Create message data
            message_data = {
                "id": i,
                "timestamp": time.time(),
                "payload": generate_random_data(payload_size)
            }
            
            
            message_json = json.dumps(message_data)
            print(f"Publishing message {i+1}/{num_messages} (size: {len(message_json)} bytes)")
            publisher.publish(topic, message_json)
            
            # Progress indicator
            if (i + 1) % 10 == 0 or i == 0:  
                print(f"Published {i + 1}/{num_messages} messages.")
            
            # Increased delay to prevent overwhelming the system
            time.sleep(0.01)  
        
        print("\nAll messages published.")
        
        # Wait a bit before closing to ensure all messages are processed
        print("Waiting for messages to be processed...")
        time.sleep(2)
        
    except KeyboardInterrupt:
        print("Shutting down publisher...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        publisher.close()
        print("Publisher closed.")

def message_handler(topic, message, user_data):
    """Handler for received messages"""
    try:
        # Parse message
        message_data = json.loads(message)
        
        
        send_time = message_data.get("timestamp", 0)
        receive_time = time.time()
        latency = (receive_time - send_time) * 1000  
        
        
        user_data["latencies"].append(latency)
        user_data["message_times"].append(receive_time)
        user_data["message_sizes"].append(len(message))
        user_data["received_count"] += 1
        
        
        if user_data["received_count"] % 10 == 0 or user_data["received_count"] == 1:  
            print(f"Received {user_data['received_count']}/{user_data['expected_count']} messages. Last latency: {latency:.2f}ms")
            
    except json.JSONDecodeError:
        print("Received invalid JSON message")
    except Exception as e:
        print(f"Error in message handler: {e}")

def run_subscriber(name, topic, num_messages):
    
    print(f"Starting Cross-IPC subscriber with name: {name}, topic: {topic}")
    print(f"Waiting for {num_messages} messages...")
    
    
    subscriber = PubSubPattern(name, DEFAULT_SHM_SIZE, verbose=True)  
    
    print("Setting up subscriber...")
    success = subscriber.setup()
    
    if not success:
        print("Failed to set up subscriber.")
        return
    
    # Prepare statistics collection
    user_data = {
        "latencies": [],
        "message_times": [],
        "message_sizes": [],
        "received_count": 0,
        "expected_count": num_messages
    }
    
    print(f"Subscribing to topic: {topic}")
    # Subscribe to the topic
    subscriber.subscribe(topic, message_handler, user_data)
    
    try:
        # Wait until all messages are received or timeout
        start_time = time.time()
        timeout = 60  # 60 seconds timeout
        
        while user_data["received_count"] < num_messages:
            
            if time.time() - start_time > timeout:
                print(f"Timeout after {timeout} seconds. Received {user_data['received_count']}/{num_messages} messages.")
                break
                
            # Sleep to avoid busy waiting
            time.sleep(0.1)
            
            
            if int(time.time()) % 5 == 0:  
                print(f"Still waiting... Received {user_data['received_count']}/{num_messages} messages.")
        
        # Calculate and print statistics
        latencies = user_data["latencies"]
        message_times = user_data["message_times"]
        message_sizes = user_data["message_sizes"]
        received_count = user_data["received_count"]
        
        if received_count == 0:
            print("No messages received. Cannot generate statistics.")
            return
            
        print("\n--- Cross-IPC Performance Statistics ---")
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
        
        # Save results to file
        results = {
            "type": "cross_ipc",
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
        
        with open("cross_ipc_results.json", "w") as f:
            json.dump(results, f, indent=2)
        print("Results saved to cross_ipc_results.json")
        
    except KeyboardInterrupt:
        print("Shutting down subscriber...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        subscriber.close()
        print("Subscriber closed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Cross-IPC Pub-Sub Benchmark")
    parser.add_argument("--mode", choices=["publisher", "subscriber"], required=True, 
                        help="Run as publisher or subscriber")
    parser.add_argument("--name", default=DEFAULT_NAME, 
                        help=f"Unique name for the pub-sub pattern (default: {DEFAULT_NAME})")
    parser.add_argument("--topic", default=DEFAULT_TOPIC, 
                        help=f"Topic name (default: {DEFAULT_TOPIC})")
    parser.add_argument("--size", type=int, default=DEFAULT_PAYLOAD_SIZE, 
                        help=f"Payload size in bytes (publisher only, default: {DEFAULT_PAYLOAD_SIZE})")
    parser.add_argument("--messages", type=int, default=DEFAULT_NUM_MESSAGES, 
                        help=f"Number of messages (default: {DEFAULT_NUM_MESSAGES})")
    
    args = parser.parse_args()
    
    if args.mode == "publisher":
        run_publisher(args.name, args.topic, args.size, args.messages)
    else:
        run_subscriber(args.name, args.topic, args.messages) 