from .transport.SharedMemory import SharedMemory
import time
import ctypes

class Lock:
    def __init__(self, id):
        """Initialize the lock using shared memory."""
        self.id = f"lock_{id}"
        # Use 4 bytes for better atomic operations
        self.shm = SharedMemory(self.id, 4)
        self.shm.setup()
        self._initialize()

    def _initialize(self):
        """Initialize lock state to 0 (unlocked)."""
        self.shm.write_bytes(ctypes.c_int32(0).value.to_bytes(4, 'little'))

    def acquire(self, blocking=True, timeout=-1):
        """
        Acquire the lock.
        
        Args:
            blocking (bool): If True, block until lock is available
            timeout (float): Maximum time to wait. -1 means infinite
        
        Returns:
            bool: True if lock was acquired, False otherwise
        """
        start_time = time.time()
        wait_time = 0.001
        max_wait = 0.1

        while True:
            # Try to atomically acquire the lock
            current = int.from_bytes(self.shm.read_bytes(), 'little')
            if current == 0:
                # Try to set to 1 (locked)
                self.shm.write_bytes(ctypes.c_int32(1).value.to_bytes(4, 'little'))
                # Verify we got it
                if int.from_bytes(self.shm.read_bytes(), 'little') == 1:
                    return True

            if not blocking:
                return False

            if timeout >= 0 and time.time() - start_time >= timeout:
                return False

            time.sleep(wait_time)
            wait_time = min(wait_time * 2, max_wait)

    def release(self):
        """Release the lock."""
        self.shm.write_bytes(ctypes.c_int32(0).value.to_bytes(4, 'little'))

    def __enter__(self):
        self.acquire()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.release()

    def close(self):
        """Close the lock and release resources."""
        try:
            self.release()
        finally:
            self.shm.close()
            self.shm.unlink()