import time
import sys
import signal
import random
import json
from cross_ipc import SharedMemory


DEFAULT_JSON = {
    "counters": [
        [42, 87, 123],           
        [15, 33, 55, 77],        
        [101, 202],              # Queue 3 with initial values
        [64, 128, 192, 255],     
        [7, 14, 21, 28, 35]      
    ],
    "max_counter": 20  
}


POLL_INTERVAL = 0.025  # seconds between increments
MAX_QUEUE_SIZE = 20  
LOCK_TIMEOUT_MS = 2000  

def main():
    print("Counter Incrementer (Producer)")
    print("This program adds random numbers to random queues in shared memory.")
    
    
    shm = SharedMemory("CounterSync", 4096, True)
    success = shm.setup()
    
    if not success:
        print("Failed to set up shared memory")
        return
    
    # Initialize JSON structure if it doesn't exist
    try:
        data_str = shm.read() or ""
        if data_str:
            try:
                data = json.loads(data_str)
                # Verify data is a dictionary
                if not isinstance(data, dict):
                    print(f"Invalid data format (not a dictionary): {data_str}")
                    data = DEFAULT_JSON
                    print("Initializing with predefined values.")
            except json.JSONDecodeError:
                print(f"Invalid JSON data: {data_str}")
                data = DEFAULT_JSON
                print("Initializing with predefined values.")
        else:
            data = DEFAULT_JSON
            print("Initializing with predefined values.")
            
        # Use locking for initial write
        if not shm.write_with_lock(json.dumps(data), LOCK_TIMEOUT_MS):
            print("Failed to acquire lock for initial write")
            return
    except Exception as e:
        print(f"Error initializing data: {e}")
        data = DEFAULT_JSON
        print("Initializing with predefined values.")
        # Use locking for initial write
        if not shm.write_with_lock(json.dumps(data), LOCK_TIMEOUT_MS):
            print("Failed to acquire lock for initial write")
            return
    
    # Print initial state of queues
    print("\nInitial queue state:")
    for i, queue in enumerate(data["counters"]):
        print(f"Queue {i+1}: {queue} ({len(queue)} items)")
    
    print(f"\nConnected to shared memory. Current data structure initialized.")
    print("\n=================================================")
    print("PRODUCER RUNNING")
    print(f"Adding items every {POLL_INTERVAL} seconds (max per queue: {MAX_QUEUE_SIZE})")
    print("Press Ctrl+C to stop")
    print("=================================================\n")
    
    try:
        while True:
            try:
                
                data_str = shm.read() or "{}"
                
                try:
                    data = json.loads(data_str)
                    
                    if not isinstance(data, dict):
                        print(f"Invalid data format (not a dictionary): {data_str}")
                        data = DEFAULT_JSON
                except json.JSONDecodeError:
                    print(f"Invalid JSON data: {data_str}")
                    data = DEFAULT_JSON
                
                
                if "counters" not in data or not isinstance(data["counters"], list):
                    print("Invalid data structure, reinitializing...")
                    data = DEFAULT_JSON
                
                
                queue_index = random.randint(0, len(data["counters"]) - 1)
                
                # Make sure the selected queue is a list
                if not isinstance(data["counters"][queue_index], list):
                    data["counters"][queue_index] = []
                
                selected_queue = data["counters"][queue_index]
                
                # Check if the queue is full
                if len(selected_queue) < MAX_QUEUE_SIZE:
                    # Generate a random number to add
                    new_value = random.randint(0, 256)
                    
                    
                    selected_queue.append(new_value)
                    data["counters"][queue_index] = selected_queue
                    
                    
                    if shm.write_with_lock(json.dumps(data), LOCK_TIMEOUT_MS):
                        print(f"Added {new_value} to queue {queue_index+1}. Queue now has {len(selected_queue)} items.")
                    else:
                        print(f"[LOCK DETECTED] Failed to acquire lock, add operation skipped - {time.strftime('%H:%M:%S')}")
                else:
                    #print(f"Queue {queue_index+1} is full ({MAX_QUEUE_SIZE} items), skipping...")
                    pass
            except Exception as e:
                print(f"Error: {e}")
                # Reset to default structure on error
                data = DEFAULT_JSON
                if shm.write_with_lock(json.dumps(data), LOCK_TIMEOUT_MS):
                    
                    pass
                else:
                    print(f"[LOCK DETECTED] Failed to acquire lock for error recovery - {time.strftime('%H:%M:%S')}")
            
            
            time.sleep(POLL_INTERVAL)
    
    except KeyboardInterrupt:
        print("\nShutting down producer...")
    
    
    shm.close()
    print("Producer closed.")

if __name__ == "__main__":
    # Handle Ctrl+C gracefully
    def signal_handler(sig, frame):
        print("\nExiting...")
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    
    
    main() 