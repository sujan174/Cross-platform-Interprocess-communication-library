#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "store_dict_pattern.h"
#include "pub_sub_pattern.h"
#include "req_resp_pattern.h"
#include "dispenser_pattern.h"
#include "shm_dispenser_pattern.h"
#include "shared_memory.h"

// Add this at the top of the file to disable the scanf warnings
#define _CRT_SECURE_NO_WARNINGS


#define PIPE_BUFFER_SIZE 4096

// Function prototypes
void StoreDictPattern_sender();
void StoreDictPattern_receiver();
void PubSubPattern_publisher();
void PubSubPattern_subscriber();
void ReqRespPattern_server();
void ReqRespPattern_client();
void DispenserPattern_provider();
void DispenserPattern_consumer();
void ShmDispenserPattern_provider();
void ShmDispenserPattern_consumer();
void SharedMemory_sync_test();

// Message handler for PubSubPattern
void message_handler(PubSubPattern* pubsub, const char* topic, const unsigned char* payload, size_t payload_size, void* user_data) {
    printf("Received message on topic '%s': ", topic);

    
    bool is_string = true;
    for (size_t i = 0; i < payload_size; i++) {
        if (payload[i] == 0) {
            is_string = true;
            break;
        }
        if (payload[i] < 32 || payload[i] > 126) {
            is_string = false;
        }
    }

    if (is_string) {
        printf("%s\n", (const char*)payload);
    }
    else {
        printf("(binary data, %zu bytes)\n", payload_size);
        // Print first 16 bytes as hex
        printf("First 16 bytes: ");
        for (size_t i = 0; i < payload_size && i < 16; i++) {
            printf("%02X ", payload[i]);
        }
        printf("\n");
    }
}

// Request handler for ReqRespPattern
char* request_handler(const char* request, void* user_data) {
    printf("Server received request: %s\n", request);
    
    
    char* response = (char*)malloc(strlen(request) + 50);
    sprintf_s(response, strlen(request) + 50, "Echo from server: %s", request);
    
    return response;
}


char* calculator_handler(const char* request, void* user_data) {
    printf("Calculator received request: %s\n", request);
    
    
    char operation[20];
    double num1, num2, result;
    int items = sscanf_s(request, "%s %lf %lf", operation, (unsigned)sizeof(operation), &num1, &num2);
    
    if (items != 3) {
        return _strdup("Error: Invalid format. Use 'operation num1 num2'");
    }
    
    if (strcmp(operation, "add") == 0) {
        result = num1 + num2;
    } else if (strcmp(operation, "subtract") == 0) {
        result = num1 - num2;
    } else if (strcmp(operation, "multiply") == 0) {
        result = num1 * num2;
    } else if (strcmp(operation, "divide") == 0) {
        if (num2 == 0) {
            return _strdup("Error: Division by zero");
        }
        result = num1 / num2;
    } else {
        return _strdup("Error: Unknown operation. Use add, subtract, multiply, or divide");
    }
    
    
    char* response = (char*)malloc(100);
    sprintf_s(response, 100, "Result: %.2f", result);
    return response;
}

int main() {
    int a = 1234;
    int choice;

    printf("Choose IPC pattern:\n");
    printf("1. Store Dictionary Pattern\n");
    printf("2. Pub/Sub Pattern\n");
    printf("3. Request-Response Pattern\n");
    printf("4. Dispenser Pattern (Named Pipes)\n");
    printf("5. Dispenser Pattern (Shared Memory)\n");
    printf("6. Shared Memory Synchronization Test\n");
    printf("Enter your choice (1-6): ");
    scanf_s("%d", &choice);
    getchar(); // Consume newline

    switch (choice) {
    case 1:
        printf("Enter 's' to send or any other key to receive: ");
        char input = getchar();
        if (input == 's') {
            StoreDictPattern_sender();
        } else {
            StoreDictPattern_receiver();
        }
        break;
    case 2:
        printf("Enter 'p' to publish or any other key to subscribe: ");
        input = getchar();
        if (input == 'p') {
            PubSubPattern_publisher();
        } else {
            PubSubPattern_subscriber();
        }
        break;
    case 3:
        printf("Enter 's' to serve or any other key to request: ");
        input = getchar();
        if (input == 's') {
            ReqRespPattern_server();
        } else {
            ReqRespPattern_client();
        }
        break;
    case 4:
        printf("Enter 'p' to provide or any other key to consume: ");
        input = getchar();
        if (input == 'p') {
            DispenserPattern_provider();
        } else {
            DispenserPattern_consumer();
        }
        break;
    case 5:
        printf("Enter 'p' to provide or any other key to consume: ");
        input = getchar();
        if (input == 'p') {
            ShmDispenserPattern_provider();
        } else {
            ShmDispenserPattern_consumer();
        }
        break;
    case 6:
        SharedMemory_sync_test();
        break;
    default:
        printf("Invalid choice\n");
        break;
    }

    return 0;
}

void StoreDictPattern_sender() {
    printf("Store Dictionary Pattern Sender function called.\n");

    // Initialize the StoreDictPattern
    StoreDictPattern store;
    StoreDictPattern_init(&store, "MySharedDict", 4096, true);

    
    printf("Setting up StoreDictPattern with id 'MySharedDict'\n");
    store.setup(&store);

    // Store some key-value pairs
    store.store_string(&store, "greeting", "Hello from the sender!");
    store.store_string(&store, "message", "This is a test message");
    store.store_string(&store, "number", "12345");

    printf("Stored 3 key-value pairs in the dictionary.\n");

    
    printf("Dictionary contains %zu keys:\n", store.entry_count);
    for (size_t i = 0; i < store.entry_count; i++) {
        printf("  %s: %s\n", store.entries[i].key, (char*)store.entries[i].value);
    }

    printf("\n=================================================\n");
    printf("IMPORTANT: KEEP THIS WINDOW OPEN!\n");
    printf("The shared memory will only exist while this program is running.\n");
    printf("Start the receiver now, then press Enter here when done.\n");
    printf("=================================================\n\n");

    getchar(); 

    
    store.close(&store);
}

void StoreDictPattern_receiver() {
    printf("Store Dictionary Pattern Receiver function called.\n");

    // Initialize the StoreDictPattern
    StoreDictPattern store;
    StoreDictPattern_init(&store, "MySharedDict", 4096, true);

    printf("Waiting for sender to create the dictionary...\n");
    printf("Make sure the sender is running!\n");
    printf("Press Enter when sender has created the dictionary...\n");
    getchar(); 

    
    printf("\nSetting up StoreDictPattern with id 'MySharedDict'\n");
    store.setup(&store);

    // List all keys
    printf("Dictionary contains %zu keys:\n", store.entry_count);
    for (size_t i = 0; i < store.entry_count; i++) {
        printf("  %s: %s\n", store.entries[i].key, (char*)store.entries[i].value);
    }

    printf("\nAdding a new key 'response'...\n");
    store.store_string(&store, "response", "Hello from the receiver!");

    
    size_t size;
    char* greeting = (char*)store.retrieve(&store, "greeting", &size);
    if (greeting) {
        printf("Retrieved greeting: %s\n", greeting);
        free(greeting);
    }
    else {
        printf("Failed to retrieve greeting.\n");
    }

    printf("\nPress Enter to exit...\n");
    getchar(); // Wait for user to press Enter

    
    store.close(&store);
}

void PubSubPattern_publisher() {
    printf("Pub/Sub Pattern Publisher function called.\n");
    
    // Initialize the PubSubPattern
    PubSubPattern pubsub;
    PubSubPattern_init(&pubsub, "MyPubSub", 4096, false);  
    
    // Set up the pub/sub system
    printf("Setting up PubSubPattern with id 'MyPubSub'\n");
    pubsub.setup(&pubsub);
    
    // Create some topics
    pubsub.create_topic(&pubsub, "news");
    pubsub.create_topic(&pubsub, "weather");
    pubsub.create_topic(&pubsub, "sports");
    
    printf("Created 3 topics: news, weather, sports\n");
    printf("\n=================================================\n");
    printf("IMPORTANT: KEEP THIS WINDOW OPEN!\n");
    printf("The pub/sub system will only exist while this program is running.\n");
    printf("Start the subscriber now, then you can publish messages here.\n");
    printf("=================================================\n\n");
    
    
    char topic[256];
    char message[1024];
    
    while (1) {
        
        pubsub.verbose = false;
        
        printf("\nEnter topic (news, weather, sports) or 'quit' to exit: ");
        fgets(topic, sizeof(topic), stdin);
        topic[strcspn(topic, "\n")] = 0; // Remove newline
        
        if (strcmp(topic, "quit") == 0) {
            break;
        }
        
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0; 
        
        // Temporarily enable verbose for publishing
        pubsub.verbose = true;
        pubsub.publish_string(&pubsub, topic, message);
        printf("Published message to topic '%s'\n", topic);
        // Turn verbose back off
        pubsub.verbose = false;
    }
    
    // Clean up
    pubsub.close(&pubsub);
}

void PubSubPattern_subscriber() {
    printf("Pub/Sub Pattern Subscriber function called.\n");
    
    
    PubSubPattern pubsub;
    PubSubPattern_init(&pubsub, "MyPubSub", 4096, false);  // Set verbose to false initially
    
    printf("Waiting for publisher to create the pub/sub system...\n");
    printf("Make sure the publisher is running!\n");
    printf("Press Enter when publisher has created the pub/sub system...\n");
    getchar(); 
    
    
    printf("\nSetting up PubSubPattern with id 'MyPubSub'\n");
    pubsub.setup(&pubsub);
    
    // Subscribe to topics
    printf("Subscribing to topics: news, weather, sports\n");
    pubsub.subscribe(&pubsub, "news", message_handler, NULL);
    pubsub.subscribe(&pubsub, "weather", message_handler, NULL);
    pubsub.subscribe(&pubsub, "sports", message_handler, NULL);
    
    // Enable verbose for receiving messages
    pubsub.verbose = true;
    
    printf("\nListening for messages. Press Enter to exit...\n");
    getchar(); 
    
    
    pubsub.close(&pubsub);
}

void ReqRespPattern_server() {
    printf("Request-Response Pattern Server function called.\n");
    
    // Initialize the ReqRespPattern
    ReqRespPattern rr;
    ReqRespPattern_init(&rr, true);
    
    
    printf("Setting up server with ID 'echo'\n");
    rr.setup_server(&rr, "echo");
    
    // Set up another server for calculator service
    printf("Setting up server with ID 'calculator'\n");
    rr.setup_server(&rr, "calculator");
    
    // Register handlers
    printf("Registering handlers\n");
    rr.respond(&rr, "echo", request_handler, NULL);
    rr.respond(&rr, "calculator", calculator_handler, NULL);
    
    printf("\n=================================================\n");
    printf("IMPORTANT: KEEP THIS WINDOW OPEN!\n");
    printf("The request-response server will only exist while this program is running.\n");
    printf("Start the client now to send requests.\n");
    printf("=================================================\n\n");
    
    printf("Server is running. Press 'q' and Enter to exit...\n");
    
    // Keep running until user enters 'q'
    char input[10];
    while (1) {
        fgets(input, sizeof(input), stdin);
        if (input[0] == 'q' || input[0] == 'Q') {
            break;
        }
        printf("Server is still running. Press 'q' and Enter to exit...\n");
    }
    
    // Clean up
    rr.close(&rr);
}

void ReqRespPattern_client() {
    printf("Request-Response Pattern Client function called.\n");
    
    
    ReqRespPattern rr;
    ReqRespPattern_init(&rr, true);
    
    printf("Waiting for server to start...\n");
    printf("Make sure the server is running!\n");
    printf("Press Enter when server has started...\n");
    getchar();
    
    
    printf("\nSetting up client for 'echo' service\n");
    rr.setup_client(&rr, "echo");
    
    printf("Setting up client for 'calculator' service\n");
    rr.setup_client(&rr, "calculator");
    
    
    char service[256];
    char request[1024];
    
    while (1) {
        printf("\nEnter service (echo, calculator) or 'quit' to exit: ");
        fgets(service, sizeof(service), stdin);
        service[strcspn(service, "\n")] = 0; // Remove newline
        
        if (strcmp(service, "quit") == 0) {
            break;
        }
        
        printf("Enter request: ");
        fgets(request, sizeof(request), stdin);
        request[strcspn(request, "\n")] = 0; // Remove newline
        
        
        char* response = rr.request(&rr, service, request);
        
        if (response) {
            printf("Received response: %s\n", response);
            free(response);
        } else {
            printf("Error: No response received\n");
        }
    }
    
    
    rr.close(&rr);
}

void DispenserPattern_provider() {
    printf("Dispenser Pattern Provider function called.\n");
    
    
    int mode_choice;
    printf("Choose dispenser mode:\n");
    printf("1. FIFO (First In, First Out - Queue)\n");
    printf("2. LIFO (Last In, First Out - Stack)\n");
    printf("3. DEQUE (Double-Ended Queue)\n");
    printf("Enter your choice (1-3): ");
    scanf_s("%d", &mode_choice);
    getchar(); 
    
    DispenserMode mode;
    switch (mode_choice) {
    case 1:
        mode = DISPENSER_MODE_FIFO;
        printf("Selected FIFO mode\n");
        break;
    case 2:
        mode = DISPENSER_MODE_LIFO;
        printf("Selected LIFO mode\n");
        break;
    case 3:
        mode = DISPENSER_MODE_DEQUE;
        printf("Selected DEQUE mode\n");
        break;
    default:
        printf("Invalid choice, defaulting to FIFO\n");
        mode = DISPENSER_MODE_FIFO;
        break;
    }
    
    
    DispenserPattern dispenser;
    DispenserPattern_init(&dispenser, "MyDispenser", mode, true);
    
    // Set up the dispenser
    printf("Setting up dispenser with id 'MyDispenser'\n");
    dispenser.setup(&dispenser);
    
    printf("\n=================================================\n");
    printf("IMPORTANT: KEEP THIS WINDOW OPEN!\n");
    printf("The dispenser will only exist while this program is running.\n");
    printf("Start the consumer now to retrieve items.\n");
    printf("=================================================\n\n");
    
    
    char item[1024];
    char command[20];
    
    while (1) {
        printf("\nEnter command (add, add_front, clear, quit): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline
        
        if (strcmp(command, "quit") == 0) {
            break;
        }
        else if (strcmp(command, "add") == 0) {
            printf("Enter item to add: ");
            fgets(item, sizeof(item), stdin);
            item[strcspn(item, "\n")] = 0; // Remove newline
            
            if (dispenser.add_string(&dispenser, item)) {
                printf("Added item: %s\n", item);
            } else {
                printf("Failed to add item\n");
            }
        }
        else if (strcmp(command, "add_front") == 0) {
            printf("Enter item to add to front: ");
            fgets(item, sizeof(item), stdin);
            item[strcspn(item, "\n")] = 0; 
            
            if (dispenser.add_string_front(&dispenser, item)) {
                printf("Added item to front: %s\n", item);
            } else {
                printf("Failed to add item to front\n");
            }
        }
        else if (strcmp(command, "clear") == 0) {
            dispenser.clear(&dispenser);
            printf("Cleared dispenser\n");
        }
        else {
            printf("Unknown command: %s\n", command);
        }
    }
    
    
    dispenser.close(&dispenser);
}

void DispenserPattern_consumer() {
    printf("Dispenser Pattern Consumer function called.\n");
    
    // Initialize the DispenserPattern with minimal setup
    char dispenser_id[256] = "MyDispenser";
    
    printf("Waiting for provider to create the dispenser...\n");
    printf("Make sure the provider is running!\n");
    printf("Press Enter when provider has started...\n");
    getchar(); // Wait for user to press Enter
    
    printf("\nAttempting to connect to dispenser '%s'...\n", dispenser_id);
    
    // Try to connect a few times
    HANDLE pipe = INVALID_HANDLE_VALUE;
    bool connected = false;
    
    for (int i = 0; i < 5; i++) {
        printf("Attempt %d/5...\n", i+1);
        
        
        char pipe_name[256];
        sprintf_s(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\dispenser_%s", dispenser_id);
        
        
        pipe = CreateFileA(
            pipe_name,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );
        
        if (pipe != INVALID_HANDLE_VALUE) {
            
            const char* cmd = "IS_EMPTY";
            DWORD bytes_written = 0;
            BOOL write_result = WriteFile(
                pipe,
                cmd,
                (DWORD)strlen(cmd),
                &bytes_written,
                NULL
            );
            
            if (write_result && bytes_written == strlen(cmd)) {
                // Read response
                char buffer[PIPE_BUFFER_SIZE];
                DWORD bytes_read = 0;
                BOOL read_result = ReadFile(
                    pipe,
                    buffer,
                    PIPE_BUFFER_SIZE - 1,
                    &bytes_read,
                    NULL
                );
                
                if (read_result && bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Connected to provider! Initial state: %s\n", 
                           strcmp(buffer, "TRUE") == 0 ? "empty" : "not empty");
                    connected = true;
                    CloseHandle(pipe);
                    break;
                }
            }
            
            CloseHandle(pipe);
        }
        
        Sleep(1000); // Wait before trying again
    }
    
    if (!connected) {
        printf("Failed to connect to provider after multiple attempts.\n");
        return;
    }
    
    
    char command[20];
    
    while (1) {
        printf("\nEnter command (dispense, dispense_back, peek, peek_back, is_empty, quit): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline
        
        if (strcmp(command, "quit") == 0) {
            break;
        }
        
        
        char pipe_name[256];
        sprintf_s(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\dispenser_%s", dispenser_id);
        
        // Connect to provider pipe
        pipe = CreateFileA(
            pipe_name,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );
        
        if (pipe == INVALID_HANDLE_VALUE) {
            printf("Failed to connect to provider: %d\n", GetLastError());
            continue;
        }
        
        // Send command
        DWORD bytes_written = 0;
        BOOL write_result = WriteFile(
            pipe,
            command,
            (DWORD)strlen(command),
            &bytes_written,
            NULL
        );
        
        if (!write_result || bytes_written != strlen(command)) {
            printf("Failed to send command: %d\n", GetLastError());
            CloseHandle(pipe);
            continue;
        }
        
        // Read response
        char buffer[PIPE_BUFFER_SIZE];
        DWORD bytes_read = 0;
        BOOL read_result = ReadFile(
            pipe,
            buffer,
            PIPE_BUFFER_SIZE - 1,
            &bytes_read,
            NULL
        );
        
        if (!read_result || bytes_read == 0) {
            printf("Failed to read response: %d\n", GetLastError());
            CloseHandle(pipe);
            continue;
        }
        
        
        buffer[bytes_read] = '\0';
        
        // Process response
        if (strncmp(buffer, "ITEM: ", 6) == 0) {
            printf("Received item: %s\n", buffer + 6);
        }
        else if (strcmp(buffer, "TRUE") == 0) {
            printf("Dispenser is empty\n");
        }
        else if (strcmp(buffer, "FALSE") == 0) {
            printf("Dispenser is not empty\n");
        }
        else if (strcmp(buffer, "OK") == 0) {
            printf("Command executed successfully\n");
        }
        else if (strncmp(buffer, "ERROR: ", 7) == 0) {
            printf("Error: %s\n", buffer + 7);
        }
        else {
            printf("Unknown response: %s\n", buffer);
        }
        
        CloseHandle(pipe);
    }
}

void ShmDispenserPattern_provider() {
    printf("Shared Memory Dispenser Pattern Provider function called.\n");
    
    // Choose mode
    int mode_choice;
    printf("Choose dispenser mode:\n");
    printf("1. FIFO (First In, First Out - Queue)\n");
    printf("2. LIFO (Last In, First Out - Stack)\n");
    printf("3. DEQUE (Double-Ended Queue)\n");
    printf("Enter your choice (1-3): ");
    scanf_s("%d", &mode_choice);
    getchar(); 
    
    ShmDispenserMode mode;
    switch (mode_choice) {
    case 1:
        mode = SHM_DISPENSER_MODE_FIFO;
        printf("Selected FIFO mode\n");
        break;
    case 2:
        mode = SHM_DISPENSER_MODE_LIFO;
        printf("Selected LIFO mode\n");
        break;
    case 3:
        mode = SHM_DISPENSER_MODE_DEQUE;
        printf("Selected DEQUE mode\n");
        break;
    default:
        printf("Invalid choice, defaulting to FIFO\n");
        mode = SHM_DISPENSER_MODE_FIFO;
        break;
    }
    
    
    ShmDispenserPattern dispenser;
    ShmDispenserPattern_init(&dispenser, "MySharedDispenser", mode, true);
    
    
    printf("Setting up shared memory dispenser with id 'MySharedDispenser'\n");
    if (!dispenser.setup(&dispenser, 16, 256)) {
        printf("Failed to set up shared memory dispenser\n");
        return;
    }
    
    printf("\n=================================================\n");
    printf("IMPORTANT: KEEP THIS WINDOW OPEN!\n");
    printf("The shared memory dispenser will only exist while this program is running.\n");
    printf("Start the consumer now to retrieve items.\n");
    printf("=================================================\n\n");
    
    // Add items in a loop
    char item[256];
    char command[20];
    
    while (1) {
        printf("\nEnter command (add, add_front, clear, quit): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline
        
        if (strcmp(command, "quit") == 0) {
            break;
        }
        else if (strcmp(command, "add") == 0) {
            printf("Enter item to add: ");
            fgets(item, sizeof(item), stdin);
            item[strcspn(item, "\n")] = 0; // Remove newline
            
            if (dispenser.add_string(&dispenser, item)) {
                printf("Added item: %s\n", item);
            } else {
                printf("Failed to add item\n");
            }
        }
        else if (strcmp(command, "add_front") == 0) {
            printf("Enter item to add to front: ");
            fgets(item, sizeof(item), stdin);
            item[strcspn(item, "\n")] = 0; 
            
            if (dispenser.add_string_front(&dispenser, item)) {
                printf("Added item to front: %s\n", item);
            } else {
                printf("Failed to add item to front\n");
            }
        }
        else if (strcmp(command, "clear") == 0) {
            dispenser.clear(&dispenser);
            printf("Cleared dispenser\n");
        }
        else {
            printf("Unknown command: %s\n", command);
        }
    }
    
    
    dispenser.close(&dispenser);
}

void ShmDispenserPattern_consumer() {
    printf("Shared Memory Dispenser Pattern Consumer function called.\n");
    
    
    ShmDispenserPattern dispenser;
    ShmDispenserPattern_init(&dispenser, "MySharedDispenser", SHM_DISPENSER_MODE_FIFO, true); // Mode doesn't matter for consumer
    
    printf("Waiting for provider to create the shared memory dispenser...\n");
    printf("Make sure the provider is running!\n");
    printf("Press Enter when provider has started...\n");
    getchar(); // Wait for user to press Enter
    
    
    printf("\nConnecting to shared memory dispenser with id 'MySharedDispenser'\n");
    if (!dispenser.setup(&dispenser, 0, 0)) { 
        printf("Failed to connect to shared memory dispenser\n");
        return;
    }
    
    printf("Connected to shared memory dispenser!\n");
    
    
    char command[20];
    
    while (1) {
        printf("\nEnter command (dispense, dispense_back, peek, peek_back, is_empty, is_full, quit): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline
        
        if (strcmp(command, "quit") == 0) {
            break;
        }
        else if (strcmp(command, "dispense") == 0) {
            char* item = dispenser.dispense_string(&dispenser);
            if (item) {
                printf("Dispensed item: %s\n", item);
                free(item);
            } else {
                printf("Dispenser is empty or operation failed\n");
            }
        }
        else if (strcmp(command, "dispense_back") == 0) {
            char* item = dispenser.dispense_string_back(&dispenser);
            if (item) {
                printf("Dispensed item from back: %s\n", item);
                free(item);
            } else {
                printf("Dispenser is empty, operation not supported in current mode, or operation failed\n");
            }
        }
        else if (strcmp(command, "peek") == 0) {
            char* item = dispenser.peek_string(&dispenser);
            if (item) {
                printf("Peeked item: %s\n", item);
                free(item);
            } else {
                printf("Dispenser is empty or operation failed\n");
            }
        }
        else if (strcmp(command, "peek_back") == 0) {
            char* item = dispenser.peek_string_back(&dispenser);
            if (item) {
                printf("Peeked item from back: %s\n", item);
                free(item);
            } else {
                printf("Dispenser is empty, operation not supported in current mode, or operation failed\n");
            }
        }
        else if (strcmp(command, "is_empty") == 0) {
            if (dispenser.is_empty(&dispenser)) {
                printf("Dispenser is empty\n");
            } else {
                printf("Dispenser is not empty\n");
            }
        }
        else if (strcmp(command, "is_full") == 0) {
            if (dispenser.is_full(&dispenser)) {
                printf("Dispenser is full\n");
            } else {
                printf("Dispenser is not full\n");
            }
        }
        else {
            printf("Unknown command: %s\n", command);
        }
    }
    
    
    dispenser.close(&dispenser);
}

void SharedMemory_sync_test() {
    printf("Shared Memory Synchronization Test\n");
    printf("This test demonstrates proper synchronization between processes.\n");
    
    printf("Choose mode:\n");
    printf("1. Writer\n");
    printf("2. Reader\n");
    printf("Enter your choice (1-2): ");
    
    char input = getchar();
    getchar(); 
    
    if (input == '1') {
        // Writer mode
        printf("Running in WRITER mode\n");
        
        // Initialize shared memory
        SharedMemory shm;
        SharedMemory_init(&shm, "SyncTest", 4096, true);
        
        
        if (!shm.setup(&shm)) {
            printf("Failed to set up shared memory\n");
            return;
        }
        
        printf("Shared memory set up. Initial value is 0.\n");
        
        
        char buffer[20];
        sprintf_s(buffer, sizeof(buffer), "%d", 0);
        if (shm.lock_for_writing(&shm, 5000)) {  
            shm.write(&shm, buffer, strlen(buffer) + 1);  // +1 for null terminator
            shm.unlock_from_writing(&shm);
        }
        else {
            printf("Failed to acquire lock for writing\n");
        }
        
        printf("\n=================================================\n");
        printf("IMPORTANT: KEEP THIS WINDOW OPEN!\n");
        printf("Start the reader now in another window.\n");
        printf("=================================================\n\n");
        
        // Increment counter in a loop
        int counter = 0;
        while (1) {
            printf("Current counter: %d. Press Enter to increment, 'q' to quit: ", counter);
            input = getchar();
            if (input == 'q' || input == 'Q') {
                break;
            }
            
            // Increment counter
            counter++;
            sprintf_s(buffer, sizeof(buffer), "%d", counter);
            
            
            if (shm.lock_for_writing(&shm, 5000)) {  
                shm.write(&shm, buffer, strlen(buffer) + 1);  
                shm.unlock_from_writing(&shm);
                printf("Incremented counter to %d\n", counter);
            }
            else {
                printf("Failed to acquire lock for writing\n");
            }
        }
        
        
        shm.close(&shm);
        printf("Writer closed\n");
    }
    else {
        
        printf("Running in READER mode\n");
        
        // Initialize shared memory
        SharedMemory shm;
        SharedMemory_init(&shm, "SyncTest", 4096, true);
        
        
        if (!shm.setup(&shm)) {
            printf("Failed to set up shared memory\n");
            return;
        }
        
        printf("Connected to shared memory.\n");
        printf("Press Enter to read the current value, 'q' to quit.\n");
        
        // Read counter in a loop
        while (1) {
            input = getchar();
            if (input == 'q' || input == 'Q') {
                break;
            }
            
            
            char* data = NULL;
            if (shm.lock_for_writing(&shm, 5000)) {
                data = shm.read(&shm);
                shm.unlock_from_writing(&shm);
            }
            else {
                printf("Failed to acquire lock for reading\n");
            }
            
            if (data) {
                printf("Current counter value: %s\n", data);
                free(data);
            }
            else {
                printf("Failed to read from shared memory\n");
            }
        }
        
        
        shm.close(&shm);
        printf("Reader closed\n");
    }
}