#include "pub_sub_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

// Helper functions
static Topic* find_topic(PubSubPattern* self, const char* topic_name);
static Topic* create_topic_internal(PubSubPattern* self, const char* topic_name);
static void add_subscriber(Topic* topic, MessageHandler handler, void* user_data);
static bool has_seen_message(Topic* topic, uint32_t msg_id);
static void add_seen_message(Topic* topic, uint32_t msg_id);
static unsigned char* pack_message(const unsigned char* payload, size_t payload_size, uint32_t msg_id, size_t* out_size);
static bool unpack_message(const unsigned char* data, size_t data_size, uint32_t* out_msg_id, unsigned char** out_payload, size_t* out_payload_size);
static unsigned __stdcall polling_thread_func(void* arg);


void PubSubPattern_init(PubSubPattern* pubsub, const char* name, size_t size, bool verbose) {
    
    StoreDictPattern_init(&pubsub->store, name, size, verbose);

    
    pubsub->topics = NULL;
    pubsub->topic_count = 0;
    pubsub->topic_capacity = 0;
    pubsub->running = false;
    pubsub->polling_thread = NULL;
    pubsub->message_counter = 0;
    pubsub->verbose = verbose;

    
    pubsub->setup = PubSubPattern_setup;
    pubsub->publish = PubSubPattern_publish;
    pubsub->publish_string = PubSubPattern_publish_string;
    pubsub->subscribe = PubSubPattern_subscribe;
    pubsub->create_topic = PubSubPattern_create_topic;
    pubsub->close = PubSubPattern_close;
}


bool PubSubPattern_setup(PubSubPattern* self) {
    // Set up the StoreDictPattern
    if (!self->store.setup(&self->store)) {
        return false;
    }

    // Start the polling thread
    self->running = true;
    self->polling_thread = (HANDLE)_beginthreadex(NULL, 0, polling_thread_func, self, 0, NULL);

    if (!self->polling_thread) {
        self->running = false;
        return false;
    }

    return true;
}

void PubSubPattern_publish(PubSubPattern* self, const char* topic, const unsigned char* message, size_t message_size) {
    // Increment message counter
    self->message_counter = (self->message_counter + 1) % 0xFFFFFFFF;

    
    size_t packed_size;
    unsigned char* packed_message = pack_message(message, message_size, self->message_counter, &packed_size);

    if (!packed_message) {
        if (self->verbose) {
            printf("Failed to pack message for topic '%s'\n", topic);
        }
        return;
    }

    
    self->store.store_bytes(&self->store, topic, packed_message, packed_size);

    // Free the packed message
    free(packed_message);

    if (self->verbose) {
        printf("Published message to topic '%s' with ID %u\n", topic, self->message_counter);
    }
}

void PubSubPattern_publish_string(PubSubPattern* self, const char* topic, const char* message) {
    self->publish(self, topic, (const unsigned char*)message, strlen(message) + 1);
}

void PubSubPattern_subscribe(PubSubPattern* self, const char* topic, MessageHandler handler, void* user_data) {
    
    Topic* topic_obj = find_topic(self, topic);
    if (!topic_obj) {
        topic_obj = create_topic_internal(self, topic);
        if (!topic_obj) {
            if (self->verbose) {
                printf("Failed to create topic '%s'\n", topic);
            }
            return;
        }
    }

    // Add the subscriber
    add_subscriber(topic_obj, handler, user_data);

    if (self->verbose) {
        printf("Subscribed to topic '%s'\n", topic);
    }
}

void PubSubPattern_create_topic(PubSubPattern* self, const char* topic) {
    
    uint32_t msg_id = 0;
    unsigned char empty_payload[1] = { 0 };  

    size_t packed_size;
    unsigned char* packed_message = pack_message(empty_payload, 1, msg_id, &packed_size);

    if (!packed_message) {
        if (self->verbose) {
            printf("Failed to pack message for topic '%s'\n", topic);
        }
        return;
    }

    
    self->store.store(&self->store, topic, packed_message, packed_size);

    
    free(packed_message);

    if (self->verbose) {
        printf("Created topic '%s'\n", topic);
    }
}

void PubSubPattern_close(PubSubPattern* self) {
    
    self->running = false;

    if (self->polling_thread) {
        WaitForSingleObject(self->polling_thread, 1000);
        CloseHandle(self->polling_thread);
        self->polling_thread = NULL;
    }

    
    self->store.close(&self->store);

    
    for (size_t i = 0; i < self->topic_count; i++) {
        free(self->topics[i].name);
        free(self->topics[i].subscribers);
        free(self->topics[i].seen_message_ids);
    }

    free(self->topics);
    self->topics = NULL;
    self->topic_count = 0;
    self->topic_capacity = 0;

    if (self->verbose) {
        printf("PubSubPattern closed\n");
    }
}


static Topic* find_topic(PubSubPattern* self, const char* topic_name) {
    for (size_t i = 0; i < self->topic_count; i++) {
        if (strcmp(self->topics[i].name, topic_name) == 0) {
            return &self->topics[i];
        }
    }
    return NULL;
}

static Topic* create_topic_internal(PubSubPattern* self, const char* topic_name) {
    
    if (self->topic_count >= self->topic_capacity) {
        size_t new_capacity = self->topic_capacity == 0 ? 4 : self->topic_capacity * 2;
        Topic* new_topics = (Topic*)realloc(self->topics, new_capacity * sizeof(Topic));

        if (!new_topics) {
            return NULL;
        }

        self->topics = new_topics;
        self->topic_capacity = new_capacity;
    }

    // Initialize the new topic
    Topic* topic = &self->topics[self->topic_count++];
    topic->name = _strdup(topic_name);
    topic->subscribers = NULL;
    topic->subscriber_count = 0;
    topic->subscriber_capacity = 0;
    topic->seen_message_ids = NULL;
    topic->seen_ids_count = 0;
    topic->seen_ids_capacity = 0;

    return topic;
}

static void add_subscriber(Topic* topic, MessageHandler handler, void* user_data) {
    // Check if we need to resize the subscribers array
    if (topic->subscriber_count >= topic->subscriber_capacity) {
        size_t new_capacity = topic->subscriber_capacity == 0 ? 4 : topic->subscriber_capacity * 2;
        Subscriber* new_subscribers = (Subscriber*)realloc(topic->subscribers, new_capacity * sizeof(Subscriber));

        if (!new_subscribers) {
            return;
        }

        topic->subscribers = new_subscribers;
        topic->subscriber_capacity = new_capacity;
    }

    
    topic->subscribers[topic->subscriber_count].handler = handler;
    topic->subscribers[topic->subscriber_count].user_data = user_data;
    topic->subscriber_count++;
}

static bool has_seen_message(Topic* topic, uint32_t msg_id) {
    for (size_t i = 0; i < topic->seen_ids_count; i++) {
        if (topic->seen_message_ids[i] == msg_id) {
            return true;
        }
    }
    return false;
}

static void add_seen_message(Topic* topic, uint32_t msg_id) {
    
    if (topic->seen_ids_count >= topic->seen_ids_capacity) {
        size_t new_capacity = topic->seen_ids_capacity == 0 ? 16 : topic->seen_ids_capacity * 2;
        uint32_t* new_ids = (uint32_t*)realloc(topic->seen_message_ids, new_capacity * sizeof(uint32_t));

        if (!new_ids) {
            return;
        }

        topic->seen_message_ids = new_ids;
        topic->seen_ids_capacity = new_capacity;
    }

    
    topic->seen_message_ids[topic->seen_ids_count++] = msg_id;

    // If we have too many IDs, remove the oldest ones
    if (topic->seen_ids_count > 1000) {
        // Keep the 500 most recent IDs
        memmove(topic->seen_message_ids, topic->seen_message_ids + 500, 500 * sizeof(uint32_t));
        topic->seen_ids_count = 500;
    }
}

static unsigned char* pack_message(const unsigned char* payload, size_t payload_size, uint32_t msg_id, size_t* out_size) {
    // Calculate the total size: msg_id (4 bytes) + size (4 bytes) + payload
    size_t total_size = 8 + payload_size;

    
    unsigned char* packed = (unsigned char*)malloc(total_size);
    if (!packed) {
        return NULL;
    }

    // Write the message ID (4 bytes)
    memcpy(packed, &msg_id, 4);

    // Write the payload size (4 bytes)
    uint32_t size32 = (uint32_t)payload_size;
    memcpy(packed + 4, &size32, 4);

    
    memcpy(packed + 8, payload, payload_size);

    
    if (out_size) {
        *out_size = total_size;
    }

    return packed;
}

static bool unpack_message(const unsigned char* data, size_t data_size, uint32_t* out_msg_id, unsigned char** out_payload, size_t* out_payload_size) {
    
    if (!data || data_size < 8) {
        return false;
    }

    
    uint32_t msg_id;
    memcpy(&msg_id, data, 4);

    
    uint32_t payload_size;
    memcpy(&payload_size, data + 4, 4);

    
    if (8 + payload_size > data_size) {
        return false;
    }

    
    unsigned char* payload = (unsigned char*)malloc(payload_size + 1);
    if (!payload) {
        return false;
    }

    
    memcpy(payload, data + 8, payload_size);
    payload[payload_size] = '\0';  // Ensure null termination

    
    if (out_msg_id) {
        *out_msg_id = msg_id;
    }

    if (out_payload) {
        *out_payload = payload;
    }
    else {
        free(payload);
    }

    if (out_payload_size) {
        *out_payload_size = payload_size;
    }

    return true;
}

static unsigned __stdcall polling_thread_func(void* arg) {
    PubSubPattern* self = (PubSubPattern*)arg;

    while (self->running) {
        
        self->store.load(&self->store);

        // Get all keys (topics)
        size_t key_count;
        char** keys = self->store.list_keys(&self->store, &key_count);

        if (keys) {
            // Process each topic
            for (size_t i = 0; i < key_count; i++) {
                const char* topic_name = keys[i];

                // Find or create the topic
                Topic* topic = find_topic(self, topic_name);
                if (!topic) {
                    topic = create_topic_internal(self, topic_name);
                    if (!topic) {
                        continue;
                    }
                }

                
                size_t data_size;
                unsigned char* data = self->store.retrieve(&self->store, topic_name, &data_size);

                if (data && data_size >= 8) {  
                    
                    uint32_t msg_id;
                    unsigned char* payload;
                    size_t payload_size;

                    if (unpack_message(data, data_size, &msg_id, &payload, &payload_size)) {
                        
                        if (!has_seen_message(topic, msg_id)) {
                            
                            for (size_t j = 0; j < topic->subscriber_count; j++) {
                                Subscriber* subscriber = &topic->subscribers[j];
                                subscriber->handler(self, topic_name, payload, payload_size, subscriber->user_data);
                            }

                            // Mark the message as seen
                            add_seen_message(topic, msg_id);

                            if (self->verbose) {
                                printf("Received message on topic '%s' with ID %u\n", topic_name, msg_id);
                            }
                        }

                        free(payload);
                    }

                    free(data);
                }
            }

            
            for (size_t i = 0; i < key_count; i++) {
                free(keys[i]);
            }
            free(keys);
        }

        // Sleep for a longer time to reduce CPU usage
        Sleep(100);  
    }

    return 0;
}