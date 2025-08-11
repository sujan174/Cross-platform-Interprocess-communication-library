# ipc_lib/message_queue.py

import struct
from ipc_lib.shared_memory_block import SharedMemoryBlock

class SharedMemoryQueue:
    HEADER_SIZE = 8  # 4 bytes for head, 4 bytes for tail

    def __init__(self, name: str, size: int = 1024, create=True):
        self.shm = SharedMemoryBlock(name, size=size, create=create)
        if create:
            self._write_header(0, 0)  # init head and tail

    def _write_header(self, head, tail):
        self.shm.write(struct.pack("II", head, tail), 0)

    def _read_header(self):
        head, tail = struct.unpack("II", self.shm.read(8, 0))
        return head, tail

    def enqueue(self, data: bytes):
        head, tail = self._read_header()
        if tail + len(data) + 4 > self.shm.size:
            raise BufferError("Queue full")

        self.shm.write(struct.pack("I", len(data)), self.HEADER_SIZE + tail)
        self.shm.write(data, self.HEADER_SIZE + tail + 4)
        tail += len(data) + 4
        self._write_header(head, tail)

    def dequeue(self) -> bytes:
        head, tail = self._read_header()
        if head == tail:
            return None  # Empty queue

        length = struct.unpack("I", self.shm.read(4, self.HEADER_SIZE + head))[0]
        data = self.shm.read(length, self.HEADER_SIZE + head + 4)
        head += length + 4
        self._write_header(head, tail)
        return data

    def close(self):
        self.shm.close()

    def unlink(self):
        self.shm.unlink()
