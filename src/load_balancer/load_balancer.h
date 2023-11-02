/* Copyright 2023 Damian Mihai-Robert */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"

#define SERVER_SIZE sizeof(*main->servers)

typedef struct load_balancer load_balancer;
struct load_balancer {
    int64_t servers_num;
    server_t *servers;
};

/**
 * init_load_balancer() - initializes the memory for a new load balancer and its fields and
 *                        returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will return via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will return the server ID
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does not exist in the system.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 2 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute all objects stored on the
 * removed server and will delete all replicas from the hash ring.
 */
void loader_remove_server(load_balancer *main, int server_id);

/**
 * find_position() - Finds the position for an object/server on the hashring.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string or the ID of a server.
 * @arg3: Pointer to a hashing function for key/ID.
 *
 * Return: The index where the new server/object should be added.
 * 
 * Note: If used for a key, the position will be the next index of the server
 * that should contain the object.
 */
int64_t find_position(load_balancer *main, void *key,
                      uint32_t (*hash_function)(void *));

/**
 * shift_right_servers() - Shifts right the servers starting at an index.
 * @arg1: Load balancer which distributes the work.
 * @arg2: The starting position of the shift.
 */
void shift_right_servers(load_balancer **main, int64_t start_position);

/**
 * add_rebalance_servers() - Rebalances the servers after adding a new one.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Index of the server to rebalance.
 * 
 * It parses the memory of the server and re-adds the objects on the
 * closest server.
 */
void add_rebalance_servers(load_balancer *main, int source);

/**
 * loader_add_replica() - Adds a new server/replica to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * This function is called 3 times by the loader_add_server() function
 * once for the new server and twice for the replicas.
 * 
 * Note: The name is loader_add_replica() because we were not allowed to
 * change loader_add_server() function name.
 */
void loader_add_replica(load_balancer *main, int server_id);

/**
 * remove_rebalance_servers() - Rebalances the servers after removing one.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Pointer to the just removed server.
 * 
 * It parses the memory of the server and re-adds the objects on the
 * closest server.
 */
void remove_rebalance_servers(load_balancer *main, server_t *source);

/**
 * loader_remove_replica() - Removes a server/replica from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the server that should be removed.
 *
 * This function is called 3 times by the loader_remove_server() function
 * once for the server and twice for the replicas.
 * 
 * Note: The name is loader_remove_replica() because we were not allowed to
 * change loader_remove_server() function name.
 */
void loader_remove_replica(load_balancer *main, int server_id);

/**
 * my_strdup() - Functionality for duplicating a string.
 * @arg1: The string that should be duplicated.
 *
 * Return: Pointer to the duplicated string.
 * 
 * I made this function because I didn't have strdup() from string.h,
 * I don't know why.
 */
char* my_strdup(const char* str);

#endif /* LOAD_BALANCER_H_ */
