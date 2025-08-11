
import time
import sys
import os
from cross_ipc import SharedMemory

def writer_mode():
    print("Running in WRITER mode")
    
    # Initialize shared memory
    shm = SharedMemory("SyncTest", 4096, True)
    success = shm.setup()
    
    if not success:
        print("Failed to set up shared memory")
        return
    
    print("Shared memory set up. Initial value is 0.")
    
    
    shm.write("0")
    
    print("\n=================================================")
    print("IMPORTANT: KEEP THIS WINDOW OPEN!")
    print("Start the reader now in another window.")
    print("=================================================\n")
    
    
    counter = 0
    while True:
        print(f"Current counter: {counter}. Press Enter to increment, 'q' to quit: ", end='')
        cmd = input()
        if cmd.lower() == 'q':
            break
        
        
        counter += 1
        shm.write(str(counter))
        print(f"Incremented counter to {counter}")
    
    # Clean up
    shm.close()
    print("Writer closed")

def reader_mode():
    print("Running in READER mode")
    
    
    shm = SharedMemory("SyncTest", 4096, True)
    success = shm.setup()
    
    if not success:
        print("Failed to set up shared memory")
        return
    
    print("Connected to shared memory.")
    print("Press Enter to read the current value, 'q' to quit.")
    
    # Read counter in a loop
    while True:
        cmd = input()
        if cmd.lower() == 'q':
            break
        
        
        data = shm.read()
        if data:
            print(f"Current counter value: {data}")
        else:
            print("Failed to read from shared memory")
    
    # Clean up
    shm.close()
    print("Reader closed")

def main():
    print("Shared Memory Synchronization Test")
    print("This test demonstrates proper synchronization between processes.")
    
    print("Choose mode:")
    print("1. Writer")
    print("2. Reader")
    choice = input("Enter your choice (1-2): ")
    
    if choice == '1':
        writer_mode()
    else:
        reader_mode()

if __name__ == "__main__":
    main()