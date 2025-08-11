#pragma once
#ifndef PUB_SUB_PATTERN_H
#define PUB_SUB_PATTERN_H

#include <stdbool.h>
#include <stdint.h>
#include "store_dict_pattern.h"

// Forward declaration for handler function type
typedef struct PubSubPattern PubSubPattern;
typedef void (*MessageHandler)(PubSubPattern* pubsub, const char* topic, const unsigned char* payload, size_t payload_size, void* user_data);

// Subscriber structure
typedef struct {
    MessageHandler handler;
    void* user_data;
} Subscriber;

// Topic structure
typedef struct {
    char* name;
    Subscriber* subscribers;
    size_t subscriber_count;
    size_t subscriber_capacity;
    uint32_t* seen_message_ids;
    size_t seen_ids_count;
    size_t seen_ids_capacity;
} Topic;

// PubSubPattern structure
typedef struct PubSubPattern {
    // Data members
    StoreDictPattern store;
    Topic* topics;
    size_t topic_count;
    size_t topic_capacity;
    bool running;
    HANDLE polling_thread;
    uint32_t message_counter;
    bool verbose;

    // Method pointers
    bool (*setup)(struct PubSubPattern* self);
    void (*publish)(struct PubSubPattern* self, const char* topic, const unsigned char* message, size_t message_size);
    void (*publish_string)(struct PubSubPattern* self, const char* topic, const char* message);
    void (*subscribe)(struct PubSubPattern* self, const char* topic, MessageHandler handler, void* user_data);
    void (*create_topic)(struct PubSubPattern* self, const char* topic);
    void (*close)(struct PubSubPattern* self);
} PubSubPattern;

// Constructor
void PubSubPattern_init(PubSubPattern* pubsub, const char* name, size_t size, bool verbose);

// Method implementations
bool PubSubPattern_setup(PubSubPattern* self);
void PubSubPattern_publish(PubSubPattern* self, const char* topic, const unsigned char* message, size_t message_size);
void PubSubPattern_publish_string(PubSubPattern* self, const char* topic, const char* message);
void PubSubPattern_subscribe(PubSubPattern* self, const char* topic, MessageHandler handler, void* user_data);
void PubSubPattern_create_topic(PubSubPattern* self, const char* topic);
void PubSubPattern_close(PubSubPattern* self);

#endif // PUB_SUB_PATTERN_H