import multiprocessing
import time


def sender(conn):
    try:
        for i in range(5):
            message = f"Message #{i}"
            conn.send(message)
            print(f"[Sender] Sent to mailbox: {message}")
            time.sleep(1)
        conn.send("STOP")
        print("[Sender] Sent STOP signal")
    except Exception as e:
        print(f"[Sender] Error: {e}")
    finally:
        conn.close()


def receiver(conn):
    mailbox = []  # Local buffer acting as mailbox
    try:
        while True:
            if conn.poll(5.0):  # Check for new messages
                message = conn.recv()
                mailbox.append(message)
                print(f"[Receiver] Stored in mailbox: {message}")
                if message == "STOP":
                    print("[Receiver] STOP signal received")
                    break

            # Process messages from mailbox
            if mailbox:
                msg = mailbox.pop(0)
                if msg == "STOP":
                    print("[Receiver] Processed STOP, exiting")
                    break
                print(f"[Receiver] Processed from mailbox: {msg}")
            time.sleep(0.5)
    except Exception as e:
        print(f"[Receiver] Error: {e}")
    finally:
        conn.close()


if __name__ == "__main__":
    print("📬 Pipe Mailbox IPC Demo")
    parent_conn, child_conn = multiprocessing.Pipe()

    sender_proc = multiprocessing.Process(target=sender, args=(child_conn,))
    receiver_proc = multiprocessing.Process(target=receiver, args=(parent_conn,))

    sender_proc.start()
    receiver_proc.start()

    sender_proc.join()
    receiver_proc.join()

    print("\n✅ Pipe Mailbox Demo Completed")