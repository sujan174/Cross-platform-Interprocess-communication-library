# main.py
from ipc_lib import SharedMemoryQueue

q = SharedMemoryQueue("queue1", size=2048, create=True)
q.enqueue(b"hello")
print(q.dequeue())
q.close()
q.unlink()
