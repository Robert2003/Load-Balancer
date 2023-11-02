/* Copyright 2023 Damian Mihai-Robert */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "load_balancer.h"
#include "server.h"
#include "../utils.h"

char* my_strdup(const char* str)
{
    size_t len = strlen(str);
    char* newstr = malloc(len + 1);
    DIE(!newstr, "strdup malloc failed");
    if (newstr == NULL) {
        return NULL;
    }
    memcpy(newstr, str, len + 1);
    return newstr;
}

unsigned int hash_function_servers(void *a)
{
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a)
{
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer *init_load_balancer()
{
    load_balancer *main_server = malloc(sizeof(*main_server));
    DIE(!main_server, "main_server malloc failed");
    main_server->servers = NULL;
    main_server->servers_num = 0;
    return main_server;
}

int64_t find_position(load_balancer *main, void *key,
                      uint32_t (*hash_function)(void *))
{
    int64_t left = 0;
    int64_t right = main->servers_num;
    int64_t m;
    int64_t server_hash = hash_function(key);

    /**
     * Binary search that finds the firs occurance of hash.
    */
    while (left < right) {
        m = (left + right) / 2;
        if (main->servers[m].hash < server_hash)
            left = m + 1;
        else
            right = m;
    }

    /**
     * Finds the position by index, so the vector remains sorted.
    */
    while (left < main->servers_num && main->servers[left].hash == server_hash
           && main->servers[left].id <= *(int *)key)
        left++;

    return left;
}

void shift_right_servers(load_balancer **main, int64_t start_position)
{
    for (int64_t i = (*main)->servers_num - 1; i > start_position; i--)
        (*main)->servers[i] = (*main)->servers[i - 1];
}

void add_rebalance_servers(load_balancer *main, int source)
{
    int server_id = 0;
    ll_node_t *node = NULL;

    source %= main->servers_num;

    for (int64_t i = 0; i < main->servers[source].memory->hmax; i++) {
        node = main->servers[source].memory->buckets[i]->head;
        while (node) {
            info node_info = *(info *)node->data;
            /**
             * key and value are duplicated, because ht_remove_entry will free
             * the node_info's <key, value>, but loader_store() needs the pair
             * to add it on another server.
            */
            char *key = my_strdup(node_info.key);
            char *value = my_strdup(node_info.value);
            node = node->next;
            server_remove(main->servers[source].memory, node_info.key);
            loader_store(main, key, value, &server_id);
            free(key);
            free(value);
        }
    }
}

void loader_add_replica(load_balancer *main, int server_id)
{
    server_t *aux_mem = NULL;\
    int64_t pos;
    /**
     * If it is the first server, memory will be allocated, else the
     * load_balancer will be extended.
    */
    if (!main->servers) {
        main->servers = malloc(SERVER_SIZE);
        DIE(!main->servers, "servers malloc failed");

        main->servers[0].memory = init_server_memory();
        main->servers[0].hash = hash_function_servers(&server_id);
        main->servers[0].id = server_id;
        main->servers_num = 1;
    } else {
        /**
         * Position of the new server.
        */
        pos = find_position(main, &server_id, hash_function_servers);
        main->servers_num++;
        aux_mem = realloc(main->servers, main->servers_num * SERVER_SIZE);
        DIE(!aux_mem, "servers realloc failed");
        main->servers = aux_mem;
        shift_right_servers(&main, pos);

        main->servers[pos].memory = init_server_memory();
        main->servers[pos].hash = hash_function_servers(&server_id);
        main->servers[pos].id = server_id;
        /**
         * Rebalance the load of the next server.
        */
        add_rebalance_servers(main, pos + 1);
    }
}

void loader_add_server(load_balancer *main, int server_id)
{
    loader_add_replica(main, server_id);
    loader_add_replica(main, (1e5) + server_id);
    loader_add_replica(main, 2 * (1e5) + server_id);
}

void remove_rebalance_servers(load_balancer *main, server_t *source)
{
    int server_id = 0;
    ll_node_t *node = NULL;

    for (int64_t i = 0; i < source->memory->hmax; i++) {
        node = source->memory->buckets[i]->head;
        while (node) {
            info node_info = *(info *)node->data;
            node = node->next;
            /**
             * Adds all the load of a removed server back to the system.
            */
            loader_store(main, node_info.key, node_info.value, &server_id);
        }
    }
}

void loader_remove_replica(load_balancer *main, int server_id)
{
    server_t *aux_mem = NULL;

    int64_t pos = find_position(main, &server_id, hash_function_servers);
    if (pos == 0)
        pos = main->servers_num - 1;
    else
        pos--;
    server_t source = main->servers[pos];

    /**
     * Removes the server from the hashring.
    */
    for (int64_t i = pos; i < main->servers_num - 1; i++)
        main->servers[i] = main->servers[i + 1];
    main->servers_num--;

    if (main->servers_num) {
        /**
         * Rebalances the load of the removed server.
        */
        remove_rebalance_servers(main, &source);
        aux_mem = realloc(main->servers, main->servers_num * SERVER_SIZE);
        DIE(!aux_mem, "servers realloc failed");
        main->servers = aux_mem;
    }

    free_server_memory(source.memory);
}

void loader_remove_server(load_balancer *main, int server_id)
{
    loader_remove_replica(main, server_id);
    loader_remove_replica(main, (1e5) + server_id);
    loader_remove_replica(main, 2 * (1e5) + server_id);
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
    /**
     * Modulo to make it cyclic.
    */
    int64_t server_position = find_position(main, key, hash_function_key);
    server_position %= main->servers_num;

    server_store(main->servers[server_position].memory, key, value);
    *server_id = main->servers[server_position].id % 100000;
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
    /**
     * Modulo to make it cyclic.
    */
    int64_t server_position = find_position(main, key, hash_function_key);
    server_position %= main->servers_num;

    *server_id = (main->servers[server_position].id) % 100000;
    return server_retrieve(main->servers[server_position].memory, key);
}

void free_load_balancer(load_balancer *main)
{
    for (int64_t i = 0; i < main->servers_num; i++) {
        free_server_memory(main->servers[i].memory);
    }
    free(main->servers);
    free(main);
    main = NULL;
}
