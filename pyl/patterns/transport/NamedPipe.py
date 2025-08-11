import win32pipe
import win32file
import pywintypes

class NamedPipe:
    def __init__(self, pipe_name):
        """
        Initialize the named pipe.
        :param pipe_name: The name of the pipe.
        """
        self.pipe_name = pipe_name
        self.pipe = None

    def create_pipe(self):
        """
        Create a named pipe for communication (server-side).
        """
        try:
            self.pipe = win32pipe.CreateNamedPipe(
                self.pipe_name,
                win32pipe.PIPE_ACCESS_DUPLEX,
                win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                1, 65536, 65536, 0, None
            )
        except pywintypes.error as e:
            print(f"Failed to create named pipe: {e}")

    def connect(self):
        """
        Connect to the named pipe (server waits for client, or client connects to server).
        """
        try:
            if self.pipe:
                # Server-side: Wait for a client to connect
                win32pipe.ConnectNamedPipe(self.pipe, None)
            else:
                # Client-side: Connect to the server's named pipe
                self.pipe = win32file.CreateFile(
                    self.pipe_name,
                    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                    0, None, win32file.OPEN_EXISTING, 0, None
                )
        except pywintypes.error as e:
            print(f"Failed to connect to named pipe: {e}")

    def send(self, message):
        """
        Send a message through the named pipe.
        :param message: The message to send.
        """
        try:
            win32file.WriteFile(self.pipe, message.encode('utf-8'))
        except pywintypes.error as e:
            print(f"Failed to send message: {e}")

    def receive(self):
        """
        Receive a message from the named pipe.
        :return: The received message.
        """
        try:
            result, data = win32file.ReadFile(self.pipe, 65536)
            return data.decode('utf-8')
        except pywintypes.error as e:
            print(f"Failed to receive message: {e}")
            return None

    def close(self):
        """
        Close the named pipe.
        """
        if self.pipe:
            win32file.CloseHandle(self.pipe)