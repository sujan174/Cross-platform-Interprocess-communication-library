import pickle
from .transport.SharedMemory import SharedMemory


class StoreDictPattern:
    def __init__(self, id, size=1024):
        """
        Initialize the store pattern.
        :param id: The identifier for the store.
        :param size: The size of the shared memory.
        """
        self.id = id
        self.size = size
        self.shared_memory = SharedMemory(self.id, self.size)
        self.data = {}

    def setup(self):
        """Set up the store and load existing data if any."""
        self.shared_memory.setup()
        self.load()

    def load(self):
        """Load (or refresh) data from shared memory into the dictionary."""
        try:
            raw_data = self.shared_memory.read_bytes()
            if not raw_data:
                return
                
            new_data = {}
            pos = 0
            while pos < len(raw_data):
                # Read key length (2 bytes)
                key_len = int.from_bytes(raw_data[pos:pos+2], 'little')
                pos += 2
                if key_len == 0:  # End of data
                    break
                
                # Read key
                key = raw_data[pos:pos+key_len].decode('utf-8')
                pos += key_len
                
                # Read value length (4 bytes)
                val_len = int.from_bytes(raw_data[pos:pos+4], 'little')
                pos += 4
                
                # Read value
                value = raw_data[pos:pos+val_len]
                pos += val_len
                
                new_data[key] = value
            self.data = new_data
        except Exception as e:
            print(f"Failed to load data from shared memory: {e}")

    def store(self, key, content):
        """Store content with a specific key."""
        if not isinstance(key, str):
            raise ValueError("Key must be a string")
        if not isinstance(content, (str, bytes)):
            raise ValueError("Content must be string or bytes")
        if isinstance(content, str):
            content = content.encode('utf-8')
        self.data[key] = content
        self._sync_to_shared_memory()

    def retrieve(self, key):
        """Always return the latest value from the shared memory."""
        self.load()  # Refresh before reading
        return self.data.get(key)

    def delete(self, key):
        """
        Delete content by its key.
        :param key: The key of the content to delete.
        """
        if key in self.data:
            del self.data[key]
            self._sync_to_shared_memory()

    def list_keys(self):
        """Reload keys before listing."""
        self.load()  # Refresh before listing
        return list(self.data.keys())

    def clear(self):
        """
        Clear all stored content.
        """
        self.data.clear()
        self._sync_to_shared_memory()

    def close(self):
        """
        Close the store and release resources.
        """
        self.shared_memory.close()
        self.shared_memory.unlink()

    def _sync_to_shared_memory(self):
        """Synchronize the dictionary to shared memory."""
        try:
            # Convert dictionary to bytes
            output = bytearray()
            for key, value in self.data.items():
                key_bytes = key.encode('utf-8')
                # Add key length (2 bytes)
                output.extend(len(key_bytes).to_bytes(2, 'little'))
                # Add key
                output.extend(key_bytes)
                # Add value length (4 bytes)
                output.extend(len(value).to_bytes(4, 'little'))
                # Add value
                if isinstance(value, str):
                    value = value.encode('utf-8')
                output.extend(value)
            
            if len(output) > self.size:
                raise ValueError("Data size exceeds shared memory size")
            
            self.shared_memory.write_bytes(output)
        except Exception as e:
            print(f"Failed to sync data to shared memory: {e}")