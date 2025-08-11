import multiprocessing
import time
from ipc_lib import (
    SharedMemoryBlock,
    SharedMemoryQueue,
    Publisher, Subscriber,
    create_named_pipe, write_to_pipe, read_from_pipe
)


# === Worker functions for multiprocessing ===

def pub_process():
    pub = Publisher("pubsub_demo")
    for i in range(3):
        msg = f"Update #{i}".encode()
        pub.publish(msg)
        print("[Publisher] Sent:", msg)
        time.sleep(1)
    pub.close()
    pub.unlink()


def sub_process():
    sub = Subscriber("pubsub_demo")
    for _ in range(5):
        msg = sub.receive()
        if msg:
            print("[Subscriber] Got:", msg.strip(b"\x00"))
        time.sleep(1)
    sub.close()


def pipe_writer():
    time.sleep(1)
    write_to_pipe("Hello via pipe!")


def pipe_mailbox_sender(conn):
    try:
        for i in range(3):
            message = f"Message #{i}"
            conn.send(message)
            print(f"[Pipe Mailbox Sender] Sent: {message}")
            time.sleep(1)
        conn.send("STOP")
        print("[Pipe Mailbox Sender] Sent STOP signal")
    except Exception as e:
        print(f"[Pipe Mailbox Sender] Error: {e}")
    finally:
        conn.close()


def pipe_mailbox_receiver(conn):
    mailbox = []  # Local buffer acting as mailbox
    try:
        while True:
            if conn.poll(5.0):  # Check for new messages
                message = conn.recv()
                mailbox.append(message)
                print(f"[Pipe Mailbox Receiver] Stored: {message}")
                if message == "STOP":
                    print("[Pipe Mailbox Receiver] STOP signal stored")
                    break

            # Process messages from mailbox
            if mailbox:
                msg = mailbox.pop(0)
                if msg == "STOP":
                    print("[Pipe Mailbox Receiver] Processed STOP, exiting")
                    break
                print(f"[Pipe Mailbox Receiver] Processed: {msg}")
            time.sleep(0.5)
    except Exception as e:
        print(f"[Pipe Mailbox Receiver] Error: {e}")
    finally:
        conn.close()


# === Demo functions ===

def test_shared_memory():
    print("\n=== Shared Memory Demo ===")
    shm = SharedMemoryBlock("demo_block", size=128, create=True)
    shm.write(b"Hello Shared Memory!")
    print("Read from shared memory:", shm.read(22))
    shm.close()
    shm.unlink()


def test_message_queue():
    print("\n=== Shared Memory Message Queue Demo ===")
    q = SharedMemoryQueue("queue_demo", size=1024, create=True)
    q.enqueue(b"First Message")
    q.enqueue(b"Second Message")
    print("Dequeued:", q.dequeue())
    print("Dequeued:", q.dequeue())
    q.close()
    q.unlink()


def test_pubsub():
    print("\n=== Publisher/Subscriber Demo ===")
    pub = multiprocessing.Process(target=pub_process)
    sub = multiprocessing.Process(target=sub_process)
    pub.start()
    sub.start()
    pub.join()
    sub.join()


def test_named_pipe():
    print("\n=== Named Pipe Demo ===")
    create_named_pipe()
    writer = multiprocessing.Process(target=pipe_writer)
    writer.start()

    print("[Reader] Waiting for pipe message...")
    msg = read_from_pipe()
    print("[Reader] Got:", msg)

    writer.join()


def test_pipe_mailbox():
    print("\n=== Pipe Mailbox Demo ===")
    parent_conn, child_conn = multiprocessing.Pipe()

    sender = multiprocessing.Process(target=pipe_mailbox_sender, args=(child_conn,))
    receiver = multiprocessing.Process(target=pipe_mailbox_receiver, args=(parent_conn,))

    sender.start()
    receiver.start()

    sender.join()
    receiver.join()

    print("[Pipe Mailbox] Demo completed")


# === Main Demo Entry Point ===

if __name__ == "__main__":
    print("📦 IPC Project Demo")

    test_shared_memory()
    test_message_queue()
    test_pubsub()
    test_named_pipe()
    test_pipe_mailbox()

    print("\n✅ All IPC mechanisms demonstrated (excluding RPC).")