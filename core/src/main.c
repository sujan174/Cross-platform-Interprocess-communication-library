#include <stdio.h>
#include "named_pipe.h"
#include "ordinary_pipe.h"
#include "shared_memory.h"

void sender();
void receiver();

int main()
{
    char ch;
    printf("Enter 's' to send or any other key to receive: ");
    scanf_s(" %c", &ch); // Added space before %c to consume any leading whitespace

    if (ch == 's')
        sender();
    else
        receiver();
}

void sender()
{
    // Implementation for sender
    printf("Sender function called.\n");
    
    // Create a named pipe
    NamedPipe pipe;
    ncreate_pipe(&pipe); // Assuming you have a function to create a named pipe

    const char *message = "Hello from sender!";
    nsend_message(&pipe, message); // Send message through the named pipe
    printf("Sent message: %s\n", message);

    nclose_pipe(&pipe); // Close the named pipe after sending
}

void receiver()
{
    // Implementation for receiver
    printf("Receiver function called.\n");
    
    // Create a named pipe
    NamedPipe pipe;
    ncreate_pipe(&pipe); // Assuming you have a function to create a named pipe

    char *received_message = nreceive_message(&pipe); // Receive message from the named pipe
    printf("Received message: %s\n", received_message); // Print received message

    nclose_pipe(&pipe); // Close the named pipe after receiving
}