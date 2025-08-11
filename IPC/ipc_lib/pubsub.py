# ipc_lib/pubsub.py

import time
from ipc_lib.shared_memory_block import SharedMemoryBlock


class Publisher:
    def __init__(self, name, size=1024):
        self.shm = SharedMemoryBlock(name, size=size, create=True)

    def publish(self, data: bytes):
        self.shm.write(data)

    def close(self):
        self.shm.close()

    def unlink(self):
        self.shm.unlink()


class Subscriber:
    def __init__(self, name, size=1024):
        self.shm = SharedMemoryBlock(name, size=size, create=False)
        self.last_data = None

    def receive(self) -> bytes:
        data = self.shm.read(self.shm.get_size())
        if data != self.last_data:
            self.last_data = data
            return data
        return None

    def close(self):
        self.shm.close()
