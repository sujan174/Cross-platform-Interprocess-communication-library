#include "cross_ipc.h"
#include "store_dict_pattern.h"
#include "pub_sub_pattern.h"
#include "dispenser_pattern.h"
#include "shm_dispenser_pattern.h"
#include "req_resp_pattern.h"
#include <stdlib.h>
#include <string.h>


CROSS_IPC_API StoreDictPattern* StoreDictPattern_create(const char* name, size_t size, bool verbose) {
    StoreDictPattern* dict = (StoreDictPattern*)malloc(sizeof(StoreDictPattern));
    if (dict) {
        StoreDictPattern_init(dict, name, size, verbose);
    }
    return dict;
}

CROSS_IPC_API void StoreDictPattern_destroy(StoreDictPattern* dict) {
    if (dict) {
        dict->close(dict);
        free(dict);
    }
}

CROSS_IPC_API bool StoreDictPattern_setup_api(StoreDictPattern* dict) {
    return dict->setup(dict);
}

CROSS_IPC_API void StoreDictPattern_store_string_api(StoreDictPattern* dict, const char* key, const char* value) {
    dict->store_string(dict, key, value);
}

CROSS_IPC_API char* StoreDictPattern_retrieve_string_api(StoreDictPattern* dict, const char* key) {
    return dict->retrieve_string(dict, key);
}

CROSS_IPC_API void StoreDictPattern_close_api(StoreDictPattern* dict) {
    dict->close(dict);
}

CROSS_IPC_API void StoreDictPattern_load_api(StoreDictPattern* dict) {
    if (dict) {
        dict->load(dict);
    }
}



typedef struct {
    MessageHandlerCallback callback;
    void* user_data;
} CallbackWrapper;


static void internal_message_handler(PubSubPattern* pubsub, const char* topic,
    const unsigned char* payload, size_t payload_size,
    void* user_data) {
    CallbackWrapper* wrapper = (CallbackWrapper*)user_data;
    if (wrapper && wrapper->callback) {
        wrapper->callback(topic, (const char*)payload, wrapper->user_data);
    }
}

CROSS_IPC_API PubSubPattern* PubSubPattern_create(const char* name, size_t size, bool verbose) {
    PubSubPattern* pubsub = (PubSubPattern*)malloc(sizeof(PubSubPattern));
    if (pubsub) {
        PubSubPattern_init(pubsub, name, size, verbose);
    }
    return pubsub;
}

CROSS_IPC_API void PubSubPattern_destroy(PubSubPattern* pubsub) {
    if (pubsub) {
        pubsub->close(pubsub);
        free(pubsub);
    }
}

CROSS_IPC_API bool PubSubPattern_setup_api(PubSubPattern* pubsub) {
    return pubsub->setup(pubsub);
}

CROSS_IPC_API void PubSubPattern_publish_string_api(PubSubPattern* pubsub, const char* topic, const char* message) {
    pubsub->publish_string(pubsub, topic, message);
}

CROSS_IPC_API void PubSubPattern_subscribe_api(PubSubPattern* pubsub, const char* topic,
    MessageHandlerCallback callback, void* user_data) {
    CallbackWrapper* wrapper = (CallbackWrapper*)malloc(sizeof(CallbackWrapper));
    wrapper->callback = callback;
    wrapper->user_data = user_data;

    pubsub->subscribe(pubsub, topic, internal_message_handler, wrapper);
}

CROSS_IPC_API void PubSubPattern_close_api(PubSubPattern* pubsub) {
    pubsub->close(pubsub);
}


CROSS_IPC_API ShmDispenserPattern* ShmDispenserPattern_create(const char* name, ShmDispenserMode mode, bool verbose) {
    ShmDispenserPattern* dispenser = (ShmDispenserPattern*)malloc(sizeof(ShmDispenserPattern));
    if (dispenser) {
        ShmDispenserPattern_init(dispenser, name, mode, verbose);
    }
    return dispenser;
}

CROSS_IPC_API void ShmDispenserPattern_destroy(ShmDispenserPattern* dispenser) {
    if (dispenser) {
        dispenser->close(dispenser);
        free(dispenser);
    }
}

CROSS_IPC_API bool ShmDispenserPattern_setup_api(ShmDispenserPattern* dispenser, size_t capacity, size_t item_size) {
    return dispenser->setup(dispenser, capacity, item_size);
}

CROSS_IPC_API bool ShmDispenserPattern_add_string_api(ShmDispenserPattern* dispenser, const char* item) {
    return dispenser->add_string(dispenser, item);
}

CROSS_IPC_API bool ShmDispenserPattern_add_string_front_api(ShmDispenserPattern* dispenser, const char* item) {
    return dispenser->add_string_front(dispenser, item);
}

CROSS_IPC_API char* ShmDispenserPattern_dispense_string_api(ShmDispenserPattern* dispenser) {
    return dispenser->dispense_string(dispenser);
}

CROSS_IPC_API char* ShmDispenserPattern_dispense_string_back_api(ShmDispenserPattern* dispenser) {
    return dispenser->dispense_string_back(dispenser);
}

CROSS_IPC_API char* ShmDispenserPattern_peek_string_api(ShmDispenserPattern* dispenser) {
    return dispenser->peek_string(dispenser);
}

CROSS_IPC_API char* ShmDispenserPattern_peek_string_back_api(ShmDispenserPattern* dispenser) {
    return dispenser->peek_string_back(dispenser);
}

CROSS_IPC_API bool ShmDispenserPattern_is_empty_api(ShmDispenserPattern* dispenser) {
    return dispenser->is_empty(dispenser);
}

CROSS_IPC_API bool ShmDispenserPattern_is_full_api(ShmDispenserPattern* dispenser) {
    return dispenser->is_full(dispenser);
}

CROSS_IPC_API void ShmDispenserPattern_clear_api(ShmDispenserPattern* dispenser) {
    dispenser->clear(dispenser);
}

CROSS_IPC_API void ShmDispenserPattern_close_api(ShmDispenserPattern* dispenser) {
    dispenser->close(dispenser);
}


CROSS_IPC_API NamedPipe* NamedPipe_create(const char* pipe_name, bool verbose) {
    NamedPipe* pipe = (NamedPipe*)malloc(sizeof(NamedPipe));
    if (pipe) {
        pipe->pipe_name = _strdup(pipe_name);
        pipe->hPipe = INVALID_HANDLE_VALUE;
        pipe->verbose = verbose;
    }
    return pipe;
}

CROSS_IPC_API void NamedPipe_destroy(NamedPipe* pipe) {
    if (pipe) {
        if (pipe->pipe_name) {
            free(pipe->pipe_name);
        }
        free(pipe);
    }
}

CROSS_IPC_API void NamedPipe_create_pipe_api(NamedPipe* pipe) {
    ncreate_pipe(pipe);
}

CROSS_IPC_API void NamedPipe_connect_pipe_api(NamedPipe* pipe) {
    nconnect_pipe(pipe);
}

CROSS_IPC_API void NamedPipe_send_message_api(NamedPipe* pipe, const char* message) {
    nsend_message(pipe, message);
}

CROSS_IPC_API char* NamedPipe_receive_message_api(NamedPipe* pipe) {
    return nreceive_message(pipe);
}

CROSS_IPC_API void NamedPipe_close_pipe_api(NamedPipe* pipe) {
    nclose_pipe(pipe);
}

// OrdinaryPipe API implementations
CROSS_IPC_API OrdinaryPipe* OrdinaryPipe_create(bool verbose) {
    OrdinaryPipe* pipe = (OrdinaryPipe*)malloc(sizeof(OrdinaryPipe));
    if (pipe) {
        pipe->hRead = INVALID_HANDLE_VALUE;
        pipe->hWrite = INVALID_HANDLE_VALUE;
        pipe->verbose = verbose;
    }
    return pipe;
}

CROSS_IPC_API void OrdinaryPipe_destroy(OrdinaryPipe* pipe) {
    if (pipe) {
        free(pipe);
    }
}

CROSS_IPC_API void OrdinaryPipe_create_pipe_api(OrdinaryPipe* pipe) {
    ocreate_pipe(pipe);
}

CROSS_IPC_API void OrdinaryPipe_send_message_api(OrdinaryPipe* pipe, const char* message) {
    osend_message(pipe, message);
}

CROSS_IPC_API char* OrdinaryPipe_receive_message_api(OrdinaryPipe* pipe) {
    return oreceive_message(pipe);
}

CROSS_IPC_API void OrdinaryPipe_close_pipe_api(OrdinaryPipe* pipe) {
    oclose_pipe(pipe);
}


CROSS_IPC_API SharedMemory* SharedMemory_create(const char* id, size_t size, bool verbose) {
    SharedMemory* shm = (SharedMemory*)malloc(sizeof(SharedMemory));
    if (shm) {
        SharedMemory_init(shm, id, size, verbose);
    }
    return shm;
}

CROSS_IPC_API void SharedMemory_destroy(SharedMemory* shm) {
    if (shm) {
        shm->close(shm);
        free(shm);
    }
}

CROSS_IPC_API bool SharedMemory_setup_api(SharedMemory* shm) {
    return shm->setup(shm);
}

CROSS_IPC_API void SharedMemory_write_api(SharedMemory* shm, const char* data) {
    size_t data_len = strlen(data) + 1;  
    shm->write(shm, data, data_len);
}

CROSS_IPC_API void SharedMemory_write_bytes_api(SharedMemory* shm, const unsigned char* data, size_t data_size) {
    shm->write_bytes(shm, data, data_size);
}

CROSS_IPC_API char* SharedMemory_read_api(SharedMemory* shm) {
    return shm->read(shm);
}

CROSS_IPC_API unsigned char* SharedMemory_read_bytes_api(SharedMemory* shm, size_t* out_size) {
    return shm->read_bytes(shm, out_size);
}

CROSS_IPC_API void SharedMemory_clear_api(SharedMemory* shm) {
    shm->clear(shm);
}

CROSS_IPC_API void SharedMemory_close_api(SharedMemory* shm) {
    shm->close(shm);
}

CROSS_IPC_API void SharedMemory_unlink_api(SharedMemory* shm) {
    shm->unlink(shm);
}


CROSS_IPC_API bool SharedMemory_lock_for_writing_api(SharedMemory* shm, DWORD timeout_ms) {
    return shm->lock_for_writing(shm, timeout_ms);
}

CROSS_IPC_API bool SharedMemory_unlock_from_writing_api(SharedMemory* shm) {
    return shm->unlock_from_writing(shm);
}


CROSS_IPC_API ReqRespPattern* ReqRespPattern_create(bool verbose) {
    ReqRespPattern* rr = (ReqRespPattern*)malloc(sizeof(ReqRespPattern));
    if (rr) {
        ReqRespPattern_init(rr, verbose);
    }
    return rr;
}

CROSS_IPC_API void ReqRespPattern_destroy(ReqRespPattern* rr) {
    if (rr) {
        ReqRespPattern_close(rr);
        free(rr);
    }
}

CROSS_IPC_API bool ReqRespPattern_setup_server_api(ReqRespPattern* rr, const char* id) {
    return ReqRespPattern_setup_server(rr, id);
}

CROSS_IPC_API bool ReqRespPattern_setup_client_api(ReqRespPattern* rr, const char* id) {
    return ReqRespPattern_setup_client(rr, id);
}

CROSS_IPC_API char* ReqRespPattern_request_api(ReqRespPattern* rr, const char* id, const char* message) {
    return ReqRespPattern_request(rr, id, message);
}

CROSS_IPC_API void ReqRespPattern_respond_api(ReqRespPattern* rr, const char* id, RequestHandlerCallback handler, void* user_data) {
    ReqRespPattern_respond(rr, id, handler, user_data);
}

CROSS_IPC_API void ReqRespPattern_close_api(ReqRespPattern* rr) {
    ReqRespPattern_close(rr);
}

