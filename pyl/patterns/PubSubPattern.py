import threading
import time
import struct
from .StoreDictPattern import StoreDictPattern

class PubSubPattern:
    def __init__(self, name="pubsub", size=1024*1024):
        self.store = StoreDictPattern(name, size)
        self.handlers = {}  # topic -> list of handler functions
        self.running = False
        self.polling_thread = None
        self.seen_messages = {}  # topic -> set of seen message IDs
        self.message_counter = 0  # Global message counter

    def setup(self):
        """Set up the Pub/Sub system"""
        self.store.setup()
        self.running = True
        self.polling_thread = threading.Thread(target=self._poll_messages)
        self.polling_thread.daemon = True
        self.polling_thread.start()

    def _pack_message(self, payload, msg_id):
        """Pack message in C-compatible format: [msg_id:4][size:4][payload]"""
        if isinstance(payload, str):
            payload = payload.encode('utf-8')
        size = len(payload)
        header = struct.pack('<II', msg_id, size)
        return header + payload

    def _unpack_message(self, data):
        """Unpack message from C-compatible format"""
        if not data or len(data) < 8:
            return None, None, None
        msg_id, size = struct.unpack('<II', data[:8])
        payload = data[8:8+size]
        return msg_id, size, payload

    def _poll_messages(self):
        """Poll for new messages in shared memory"""
        while self.running:
            try:
                # Force a refresh of the store
                self.store.load()
                
                for topic in self.store.list_keys():
                    if topic not in self.seen_messages:
                        self.seen_messages[topic] = set()

                    data = self.store.retrieve(topic)
                    if not data:
                        continue

                    msg_id, size, payload = self._unpack_message(data)
                    if msg_id is not None and msg_id not in self.seen_messages[topic]:
                        if topic in self.handlers:
                            for handler in self.handlers[topic]:
                                try:
                                    handler(payload)
                                except Exception as e:
                                    print(f"Handler error: {e}")
                            self.seen_messages[topic].add(msg_id)
            except Exception as e:
                print(f"Polling error: {e}")
            time.sleep(0.01)

    def publish(self, topic, message):
        """Publish a message to a topic"""
        try:
            # Use incremental counter instead of timestamp
            self.message_counter = (self.message_counter + 1) % 0xFFFFFFFF  # Keep within uint32 range
            packed_message = self._pack_message(message, self.message_counter)
            self.store.store(topic, packed_message)
        except Exception as e:
            print(f"Publishing error: {e}")

    def subscribe(self, topic, handler):
        """Subscribe to a topic"""
        if topic not in self.handlers:
            self.handlers[topic] = []
        if handler not in self.handlers[topic]:
            self.handlers[topic].append(handler)
            if topic not in self.seen_messages:
                self.seen_messages[topic] = set()

    def create_topic(self, topic):
        """Create a new topic"""
        self.store.store(topic, self._pack_message("", 0))
        self.seen_messages[topic] = set()

    def close(self):
        """Close the Pub/Sub system"""
        self.running = False
        if self.polling_thread:
            self.polling_thread.join(timeout=1.0)
        self.store.close()
