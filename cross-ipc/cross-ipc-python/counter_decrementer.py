import time
import sys
import signal
import random
import json
from cross_ipc import SharedMemory


DEFAULT_JSON = {
    "counters": [
        [],  
        [],  # Queue 2
        [],  # Queue 3
        [],  
        []   
    ],
    "max_counter": 20  # Maximum items per queue
}

# Constants
POLL_INTERVAL = 0.025  # seconds between decrements (slightly slower than incrementer)
LOCK_TIMEOUT_MS = 2000  

def main():
    print("Counter Decrementer (Consumer)")
    print("This program removes numbers from random queues in shared memory.")
    
    # Initialize shared memory
    shm = SharedMemory("CounterSync", 4096, True)
    success = shm.setup()
    
    if not success:
        print("Failed to set up shared memory")
        return
    
    # Read current data structure
    try:
        data_str = shm.read() or ""
        if not data_str:
            print("Failed to read data. Make sure the producer is running first.")
            return
        data = json.loads(data_str)
    except Exception as e:
        print(f"Failed to read data: {e}. Make sure the producer is running first.")
        return
    
    print(f"Connected to shared memory. Current data structure loaded.")
    print("\n=================================================")
    print("CONSUMER RUNNING")
    print(f"Removing items every {POLL_INTERVAL} seconds")
    print("Press Ctrl+C to stop")
    print("=================================================\n")
    
    try:
        while True:
            try:
                
                data_str = shm.read() or "{}"
                data = json.loads(data_str)
                
                
                if "counters" not in data:
                    print("Invalid data structure, waiting for producer...")
                    time.sleep(POLL_INTERVAL)
                    continue
                
                
                non_empty_queues = [i for i, queue in enumerate(data["counters"]) if queue]
                
                if non_empty_queues:
                    
                    queue_index = random.choice(non_empty_queues)
                    selected_queue = data["counters"][queue_index]
                    
                    # Remove a value from the queue (FIFO)
                    removed_value = selected_queue.pop(0)
                    data["counters"][queue_index] = selected_queue
                    
                    # Write back with locking
                    if shm.write_with_lock(json.dumps(data), LOCK_TIMEOUT_MS):
                        print(f"Removed {removed_value} from queue {queue_index+1}. Queue now has {len(selected_queue)} items.")
                    else:
                        print(f"[LOCK DETECTED] Failed to acquire lock, remove operation skipped - {time.strftime('%H:%M:%S')}")
                else:
                    print("All queues are empty, waiting...")
            except Exception as e:
                print(f"Error: {e}")
            
            
            time.sleep(POLL_INTERVAL)
    
    except KeyboardInterrupt:
        print("\nShutting down consumer...")
    
    
    shm.close()
    print("Consumer closed.")

if __name__ == "__main__":
    # Handle Ctrl+C gracefully
    def signal_handler(sig, frame):
        print("\nExiting...")
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    
    
    main() 