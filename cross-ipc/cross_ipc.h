#pragma once
#ifndef CROSS_IPC_H
#define CROSS_IPC_H

#include <stdbool.h>
#include <stddef.h>

// Include the necessary headers
#include "named_pipe.h"
#include "ordinary_pipe.h"
#include "shared_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CROSS_IPC_EXPORTS
#define CROSS_IPC_API __declspec(dllexport)
#else
#define CROSS_IPC_API __declspec(dllimport)
#endif

	// StoreDictPattern API
	typedef struct StoreDictPattern StoreDictPattern;

	CROSS_IPC_API StoreDictPattern* StoreDictPattern_create(const char* name, size_t size, bool verbose);
	CROSS_IPC_API void StoreDictPattern_destroy(StoreDictPattern* dict);
	CROSS_IPC_API bool StoreDictPattern_setup_api(StoreDictPattern* dict);
	CROSS_IPC_API void StoreDictPattern_store_string_api(StoreDictPattern* dict, const char* key, const char* value);
	CROSS_IPC_API char* StoreDictPattern_retrieve_string_api(StoreDictPattern* dict, const char* key);
	CROSS_IPC_API void StoreDictPattern_load_api(StoreDictPattern* dict);
	CROSS_IPC_API void StoreDictPattern_close_api(StoreDictPattern* dict);

	// PubSubPattern API
	typedef struct PubSubPattern PubSubPattern;
	typedef void (*MessageHandlerCallback)(const char* topic, const char* payload, void* user_data);

	CROSS_IPC_API PubSubPattern* PubSubPattern_create(const char* name, size_t size, bool verbose);
	CROSS_IPC_API void PubSubPattern_destroy(PubSubPattern* pubsub);
	CROSS_IPC_API bool PubSubPattern_setup_api(PubSubPattern* pubsub);
	CROSS_IPC_API void PubSubPattern_publish_string_api(PubSubPattern* pubsub, const char* topic, const char* message);
	CROSS_IPC_API void PubSubPattern_subscribe_api(PubSubPattern* pubsub, const char* topic, MessageHandlerCallback callback, void* user_data);
	CROSS_IPC_API void PubSubPattern_close_api(PubSubPattern* pubsub);

	// ShmDispenserPattern API
	typedef struct ShmDispenserPattern ShmDispenserPattern;

	// Dispenser modes
	typedef enum {
		SHM_DISPENSER_MODE_FIFO,  // First In, First Out (queue)
		SHM_DISPENSER_MODE_LIFO,  // Last In, First Out (stack)
		SHM_DISPENSER_MODE_DEQUE  // Double-ended queue (can add/remove from both ends)
	} ShmDispenserMode;

	CROSS_IPC_API ShmDispenserPattern* ShmDispenserPattern_create(const char* name, ShmDispenserMode mode, bool verbose);
	CROSS_IPC_API void ShmDispenserPattern_destroy(ShmDispenserPattern* dispenser);
	CROSS_IPC_API bool ShmDispenserPattern_setup_api(ShmDispenserPattern* dispenser, size_t capacity, size_t item_size);
	CROSS_IPC_API bool ShmDispenserPattern_add_string_api(ShmDispenserPattern* dispenser, const char* item);
	CROSS_IPC_API bool ShmDispenserPattern_add_string_front_api(ShmDispenserPattern* dispenser, const char* item);
	CROSS_IPC_API char* ShmDispenserPattern_dispense_string_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API char* ShmDispenserPattern_dispense_string_back_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API char* ShmDispenserPattern_peek_string_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API char* ShmDispenserPattern_peek_string_back_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API bool ShmDispenserPattern_is_empty_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API bool ShmDispenserPattern_is_full_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API void ShmDispenserPattern_clear_api(ShmDispenserPattern* dispenser);
	CROSS_IPC_API void ShmDispenserPattern_close_api(ShmDispenserPattern* dispenser);

	// NamedPipe API
	CROSS_IPC_API NamedPipe* NamedPipe_create(const char* pipe_name, bool verbose);
	CROSS_IPC_API void NamedPipe_destroy(NamedPipe* pipe);
	CROSS_IPC_API void NamedPipe_create_pipe_api(NamedPipe* pipe);
	CROSS_IPC_API void NamedPipe_connect_pipe_api(NamedPipe* pipe);
	CROSS_IPC_API void NamedPipe_send_message_api(NamedPipe* pipe, const char* message);
	CROSS_IPC_API char* NamedPipe_receive_message_api(NamedPipe* pipe);
	CROSS_IPC_API void NamedPipe_close_pipe_api(NamedPipe* pipe);

	// OrdinaryPipe API
	CROSS_IPC_API OrdinaryPipe* OrdinaryPipe_create(bool verbose);
	CROSS_IPC_API void OrdinaryPipe_destroy(OrdinaryPipe* pipe);
	CROSS_IPC_API void OrdinaryPipe_create_pipe_api(OrdinaryPipe* pipe);
	CROSS_IPC_API void OrdinaryPipe_send_message_api(OrdinaryPipe* pipe, const char* message);
	CROSS_IPC_API char* OrdinaryPipe_receive_message_api(OrdinaryPipe* pipe);
	CROSS_IPC_API void OrdinaryPipe_close_pipe_api(OrdinaryPipe* pipe);

	// SharedMemory API
	CROSS_IPC_API SharedMemory* SharedMemory_create(const char* id, size_t size, bool verbose);
	CROSS_IPC_API void SharedMemory_destroy(SharedMemory* shm);
	CROSS_IPC_API bool SharedMemory_setup_api(SharedMemory* shm);
	CROSS_IPC_API void SharedMemory_write_api(SharedMemory* shm, const char* data);
	CROSS_IPC_API void SharedMemory_write_bytes_api(SharedMemory* shm, const unsigned char* data, size_t data_size);
	CROSS_IPC_API char* SharedMemory_read_api(SharedMemory* shm);
	CROSS_IPC_API unsigned char* SharedMemory_read_bytes_api(SharedMemory* shm, size_t* out_size);
	CROSS_IPC_API void SharedMemory_clear_api(SharedMemory* shm);
	CROSS_IPC_API void SharedMemory_close_api(SharedMemory* shm);
	CROSS_IPC_API void SharedMemory_unlink_api(SharedMemory* shm);
	
	// New lock-related API functions for SharedMemory
	CROSS_IPC_API bool SharedMemory_lock_for_writing_api(SharedMemory* shm, DWORD timeout_ms);
	CROSS_IPC_API bool SharedMemory_unlock_from_writing_api(SharedMemory* shm);

	// ReqRespPattern API
	typedef struct ReqRespPattern ReqRespPattern;
	typedef char* (*RequestHandlerCallback)(const char* request, void* user_data);

	CROSS_IPC_API ReqRespPattern* ReqRespPattern_create(bool verbose);
	CROSS_IPC_API void ReqRespPattern_destroy(ReqRespPattern* rr);
	CROSS_IPC_API bool ReqRespPattern_setup_server_api(ReqRespPattern* rr, const char* id);
	CROSS_IPC_API bool ReqRespPattern_setup_client_api(ReqRespPattern* rr, const char* id);
	CROSS_IPC_API char* ReqRespPattern_request_api(ReqRespPattern* rr, const char* id, const char* message);
	CROSS_IPC_API void ReqRespPattern_respond_api(ReqRespPattern* rr, const char* id, RequestHandlerCallback handler, void* user_data);
	CROSS_IPC_API void ReqRespPattern_close_api(ReqRespPattern* rr);

#ifdef __cplusplus
}
#endif

#endif // CROSS_IPC_H