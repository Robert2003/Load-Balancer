/* Copyright 2023 Damian Mihai-Robert */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../my_lib/map.h"
#include "server.h"

server_memory *init_server_memory()
{
	server_memory *server_mem = NULL;
	server_mem = ht_create(HMAX, hash_function_string,
								   compare_function_strings,
								   key_val_free_function);
	return server_mem;
}

void server_store(server_memory *server, char *key, char *value) {
	ht_put(server, key, strlen(key) + 1, value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
	return ht_get(server, key);
}

void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server, key);
}

void free_server_memory(server_memory *server) {
	ht_free(server);
}
