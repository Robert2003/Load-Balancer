/* Copyright 2023 Damian Mihai-Robert */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "map.h"
#include "utils.h"

int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

unsigned int hash_function_int(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_string(void *a)
{
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

void key_val_free_function(void *data)
{
	info *node_info = data;

	free(node_info->key);
	node_info->key = NULL;
	free(node_info->value);
	node_info->value = NULL;
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
					   int (*compare_function)(void*, void*),
					   void (*key_val_free_function)(void*))
{
	hashtable_t *ht = malloc(sizeof(hashtable_t));
	DIE(!ht, "Hashtable malloc failed");

	ht->buckets = malloc(hmax * sizeof(linked_list_t *));
	DIE(!ht->buckets, "Buckets malloc failed");
	for (size_t i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(info));
	}

	ht->hmax = hmax;
	ht->size = 0;

	ht->compare_function = compare_function;
	ht->hash_function = hash_function;
	ht->key_val_free_function = key_val_free_function;

	return ht;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int hash = ht->hash_function(key) % ht->hmax;

	ll_node_t *curr = ht->buckets[hash]->head;

	for (size_t i = 0; i < ht->buckets[hash]->size; i++) {
		info *node_info = curr->data;
		if(!ht->compare_function(key, node_info->key))
			return 1;
		curr = curr->next;
	}

	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	unsigned int hash = ht->hash_function(key)  % ht->hmax;

	ll_node_t *curr = ht->buckets[hash]->head;

	for (size_t i = 0; i < ht->buckets[hash]->size; i++) {
			info *node_info = curr->data;
			if(!ht->compare_function(key, node_info->key))
				return node_info->value;
			curr = curr->next;
		}

	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
			void *value, unsigned int value_size)
{
	void *old_value = NULL;
	info new_node;
	unsigned int hash = ht->hash_function(key)  % ht->hmax;

	if (ht_has_key(ht, key)) {
		old_value = ht_get(ht, key);
		if (ht->compare_function(old_value, value)) {
			free(old_value);
			old_value = NULL;
			old_value = malloc(value_size);
			DIE(!old_value, "old_value malloc failed");
			memcpy(old_value, value, value_size);
		}
	} else {
		new_node.key = malloc(key_size);
		DIE(!new_node.key, "key malloc failed");
		new_node.value = malloc(value_size);
		DIE(!new_node.value, "value malloc failed");

		memcpy(new_node.key, key, key_size);
		memcpy(new_node.value, value, value_size);

		ll_add_nth_node(ht->buckets[hash], 0, &new_node);
		ht->size++;
	}
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int hash = ht->hash_function(key)  % ht->hmax;
	ll_node_t *curr = ht->buckets[hash]->head;
	ll_node_t *currNode = NULL;
	int cnt = 0;

	if (ht_has_key(ht, key)) {
		for (size_t i = 0; i < ht->buckets[hash]->size; i++) {
			info *node_info = curr->data;
			if(!ht->compare_function(key, node_info->key)) {
				key_val_free_function(curr->data);
				currNode = ll_remove_nth_node(ht->buckets[hash], cnt);

				free(currNode->data);
				currNode->data = NULL;
				free(currNode);
				currNode = NULL;

				ht->size--;
				return;
			}
			curr = curr->next;
			cnt++;
		}
	}
}

void ht_free(hashtable_t *ht)
{
	ll_node_t *curr = NULL;

    for (size_t i = 0; i < ht->hmax; i++) {
		curr = ht->buckets[i]->head;
		for (size_t j = 0; j < ht->buckets[i]->size; j++) {
			key_val_free_function(curr->data);
			curr = curr->next;
		}
		ll_free(&ht->buckets[i]);
	}
	free(ht->buckets);
	ht->buckets = NULL;
	free(ht);
	ht = NULL;
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
