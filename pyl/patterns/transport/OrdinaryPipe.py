import os
import win32pipe
import win32file
import pywintypes

class OrdinaryPipe:
    def __init__(self, id):
        """
        Initialize the ordinary pipe.
        :param id: The identifier for the pipe.
        """
        self.id = id
        self.pipe_read = None
        self.pipe_write = None

    def create_pipe(self):
        """
        Create an ordinary pipe for communication.
        """
        try:
            self.pipe_read, self.pipe_write = os.pipe()
        except OSError as e:
            print(f"Failed to create ordinary pipe: {e}")

    def send(self, message):
        """
        Send a message through the pipe.
        :param message: The message to send.
        """
        try:
            os.write(self.pipe_write, message.encode('utf-8'))
        except OSError as e:
            print(f"Failed to send message: {e}")

    def receive(self):
        """
        Receive a message from the pipe.
        :return: The received message.
        """
        try:
            data = os.read(self.pipe_read, 65536)
            return data.decode('utf-8')
        except OSError as e:
            print(f"Failed to receive message: {e}")
            return None

    def close(self):
        """
        Close the pipe.
        """
        try:
            if self.pipe_read:
                os.close(self.pipe_read)
            if self.pipe_write:
                os.close(self.pipe_write)
        except OSError as e:
            print(f"Failed to close pipe: {e}")