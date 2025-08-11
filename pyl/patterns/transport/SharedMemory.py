from multiprocessing import shared_memory, Lock  # Change this import

class SharedMemory:
    def __init__(self, id, size, verbose=False):
        """
        Initialize the shared memory.
        :param id: The identifier for the shared memory.
        :param size: The size of the shared memory.
        """
        self.id = id
        self.size = size
        self.shm = None
        self.verbose = verbose
        self.lock = Lock()  # Change to multiprocessing.Lock

    def setup(self):
        """
        Set up the shared memory.
        """
        try:
            self.shm = shared_memory.SharedMemory(name=self.id, create=True, size=self.size)
            if self.verbose:
                print(f"Shared memory '{self.id}' created with size {self.size}.")
        except FileExistsError:
            if self.verbose:
                print(f"Shared memory '{self.id}' already exists. Attempting to attach.")
            self.shm = shared_memory.SharedMemory(name=self.id)

    def write(self, data):
        """
        Write data to the shared memory.
        :param data: The data to write (must be a string or bytes).
        """
        if isinstance(data, str):
            data = data.encode('utf-8')
        if len(data) > self.size:
            raise ValueError("Data size exceeds shared memory size.")
        self.shm.buf[:len(data)] = data

        if self.verbose:
            print(f"Data written to shared memory '{self.id}': {data.decode('utf-8')}")

    def write_bytes(self, data):
        """Write raw bytes to the shared memory."""
        if len(data) > self.size:
            raise ValueError("Data size exceeds shared memory size.")
        with self.lock:
            self.shm.buf[:len(data)] = data
            self.shm.buf[len(data):] = b'\x00' * (self.size - len(data))

    def read(self):
        """
        Read data from the shared memory.
        :return: The data read from the shared memory as a string.
        """
        data = bytes(self.shm.buf[:self.size]).rstrip(b'\x00')

        if self.verbose:
            print(f"Data read from shared memory '{self.id}': {data.decode('utf-8')}")
        return data.decode('utf-8')

    def read_bytes(self):
        """Read raw bytes from the shared memory."""
        with self.lock:
            return bytes(self.shm.buf[:self.size]).rstrip(b'\x00')

    def clear(self):
        """
        Clear the shared memory.
        """
        self.shm.buf[:self.size] = b'\x00' * self.size

    def close(self):
        """
        Close the shared memory and release resources.
        """
        if self.shm:
            self.shm.close()
            if self.verbose:
                print(f"Shared memory '{self.id}' closed.")

    def unlink(self):
        """
        Unlink the shared memory (remove it from the system).
        """
        if self.shm:
            self.shm.unlink()
            if self.verbose:
                print(f"Shared memory '{self.id}' unlinked.")
