#include "hash_table.h"

#ifdef DEBUG

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#endif

#include <malloc.h>
#include <string.h> // memcmp

#define MIN(A, B) A > B ? B : A
#define MAX(A, B) A < B ? B : A

#define HT_OVERFLOW(T) (((float)T->objects + T->deleted) / T->size) > T->max_occupancy
#define HT_OVERFLOW_NEW_SIZE(T) T->objects * 2
#define HT_OBJ(T, H) T->table[H]

HT_INT is_prime(HT_INT N) {
	if (N == 2 || N == 3 || N == 5 || N == 7) return 1;
	if (N % 2 == 0) return 0;

	for (HT_INT i = 9; i * i < N; i += 2)
		if (N % i == 0) return 0;

	return 1;
}

HT_INT next_prime(HT_INT N) {
	if (N < 2) return 2;

	HT_INT M = N;

	while (!is_prime(++M));

	return M <= N ? 0 : M;
}

HashTable* NewHashTable(HT_INT size,
						HASH(*first_hash_function) (void*),
						HASH(*second_hash_function)(void*),
						float max_occupancy) {

	HashTable *table = calloc(1, sizeof(HashTable));
	if (!table) return NULL;

	if (size < MIN_TABLE_SIZE) size = MIN_TABLE_SIZE;
	
	table->table = calloc(size, sizeof(HTObject*));
	if (!table->table) {
		free(table);
		return NULL;
	}

	if (!first_hash_function) table->first_hash_function = DEFAULT_FIRST_HASH;
	else table->first_hash_function = first_hash_function;

	if (!second_hash_function) table->first_hash_function = DEFAULT_SECOND_HASH;
	else table->second_hash_function = second_hash_function;

	if (max_occupancy > OCCUPANCY_MAX || max_occupancy < OCCUPANCY_MIN)
		table->max_occupancy = DEFAULT_OCCUPANCY;
	else table->max_occupancy = max_occupancy;

	return table;
}


HT_INT HashTableAdd(HashTable* table,
					void* key,
					HT_INT key_size,
					void* data) {

	if (!table || !key || !key_size) return 0;

	if (HT_OVERFLOW(table))
		if (HashTableResize(table, HT_OVERFLOW_NEW_SIZE(table))) return 0;

	HASH hash = table->first_hash_function(key, key_size),
		 second_hash = table->second_hash_function(key, key_size);

	HTObject* obj = calloc(1, sizeof(HTObject));
	if (!obj) return 0;

	obj->data = data;
	obj->first_hash = hash;
	obj->second_hash = second_hash;

	/*
	* The key is copied to avoid changes by a pointer.
	* 
	*/
	obj->key = calloc(1, key_size);
	if (!obj->key) return 0;
	memcpy_s(obj->key, key_size, key, key_size);

	obj->key_size = key_size;

	while (HT_OBJ(table, hash)) hash = (hash + second_hash) % table->size;

	table->table[hash] = obj;

	return hash;
}

void* HashTableRemove(HashTable* table,
					void* key,
					HT_INT key_size) {

	if (!table || !key || !key_size) return NULL;

	HASH first_hash = table->first_hash_function(key, key_size),
		 second_hash = table->second_hash_function(key, key_size);

	HASH hash = first_hash;
	HTObject* obj = NULL;

	while (1) {
		if (!HT_OBJ(table, hash)) return NULL;
		else
			if (HT_OBJ(table, hash)->first_hash == first_hash &&
				HT_OBJ(table, hash)->second_hash == second_hash &&
				HT_OBJ(table, hash)->key_size == key_size) 
				if (!memcmp(HT_OBJ(table, hash)->key, key, key_size)) {
					obj = HT_OBJ(table, hash);
					break;
				}
		hash = (hash + second_hash) % table->size;
	}

	if (!obj) return NULL;

	void* data = obj->data;

	free(obj->key);
	free(obj);
	return data;
}

void* HashTableFind(HashTable* table,
	void* key,
	HT_INT key_size) {

	if (!table || !key || !key_size) return NULL;

	HASH first_hash = table->first_hash_function(key, key_size),
		second_hash = table->second_hash_function(key, key_size);

	HASH hash = first_hash;
	HTObject* obj = NULL;

	while (1) {
		if (!HT_OBJ(table, hash)) return NULL;
		else
			if (HT_OBJ(table, hash)->first_hash == first_hash &&
				HT_OBJ(table, hash)->second_hash == second_hash &&
				HT_OBJ(table, hash)->key_size == key_size)
				if (!memcmp(HT_OBJ(table, hash)->key, key, key_size)) {
					obj = HT_OBJ(table, hash);
					break;
				}
		hash = (hash + second_hash) % table->size;
	}

	if (!obj) return NULL;

	void* data = obj->data;

	return data;
}

int main() {
	int n = 0;
	while (1) {
		scanf("%d", &n);
		printf("%d\n", next_prime(n));
	}
}

#undef HT_OVERFLOW
#undef MIN
#undef MAX
#undef _CRT_SECURE_NO_WARNINGS
