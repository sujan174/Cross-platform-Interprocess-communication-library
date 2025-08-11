from multiprocessing import shared_memory
import ctypes


class SharedMemoryBlock:
    def __init__(self, name: str, size: int = 1024, create: bool = True):
        self.name = name
        self.size = size
        if create:
            self.shm = shared_memory.SharedMemory(name=name, create=True, size=size)
        else:
            self.shm = shared_memory.SharedMemory(name=name)

    def write(self, data: bytes, offset=0):
        end = offset + len(data)
        if end > self.size:
            raise ValueError("Data exceeds shared memory size.")
        self.shm.buf[offset:end] = data

    def read(self, size: int, offset=0) -> bytes:
        return bytes(self.shm.buf[offset:offset+size])

    def close(self):
        self.shm.close()

    def unlink(self):
        self.shm.unlink()

    def pointer(self):
        return ctypes.addressof(ctypes.c_char.from_buffer(self.shm.buf))

    def get_size(self):
        return self.size
