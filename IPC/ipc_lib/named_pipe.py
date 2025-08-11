# ipc_lib/named_pipe.py

import os
import platform

PIPE_NAME = "/tmp/ipc_pipe" if platform.system() != "Windows" else r"\\.\pipe\ipc_pipe"

def create_named_pipe():
    if platform.system() != "Windows":
        if not os.path.exists(PIPE_NAME):
            os.mkfifo(PIPE_NAME)

def write_to_pipe(data: str):
    with open(PIPE_NAME, 'w') as fifo:
        fifo.write(data)

def read_from_pipe():
    with open(PIPE_NAME, 'r') as fifo:
        return fifo.read()
