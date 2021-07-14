#include "hash_table.h"

#ifdef DEBUG

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#endif

#include <malloc.h>
#include <string.h> // memcmp

#define MIN(A, B) A > B ? B : A
#define MAX(A, B) A < B ? B : A

HT_INT is_prime(HT_INT N) {
	if (N == 2 || N == 3 || N == 5 || N == 7) return 1;
	if (N % 2 == 0) return 0;

	for (HT_INT i = 3; i * i < N; i += 2)
		if (N % i == 0) return 0;

	return 1;
}

HT_INT next_prime(HT_INT N) {
	if (N < 2) return 2;

	HT_INT M = N;

	while (!is_prime(++M));

	return M <= N ? 0 : M;
}

#define HT_OVERFLOW(T) (((float)T->objects + T->deleted) / T->size) > T->max_occupancy
#define HT_OVERFLOW_NEW_SIZE(T) (T->size - T->deleted) * 2

#define HT_UNDERFLOW(T) ((float)T->objects / T->size) < (T->max_occupancy / 2)
#define HT_UNDERFLOW_NEW_SIZE(T) next_prime(T->objects / 2 + 1)

#define HT_OBJ(T, H) T->table[H]

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

	if (!second_hash_function) table->second_hash_function = DEFAULT_SECOND_HASH;
	else table->second_hash_function = second_hash_function;

	if (max_occupancy > OCCUPANCY_MAX || max_occupancy < OCCUPANCY_MIN)
		table->max_occupancy = DEFAULT_OCCUPANCY;
	else table->max_occupancy = max_occupancy;

	table->size = size;

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
	table->objects++;

	return hash;
}

HTObject* _HashTableFindObject(HashTable *table, void* key, HT_INT key_size) {
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

	if (!obj || !obj->key) return NULL;

	return obj;
}

void* HashTableRemove(HashTable* table,
					void* key,
					HT_INT key_size) {

	if (!table || !key || !key_size) return NULL;

	HTObject* obj = _HashTableFindObject(table, key, key_size);

	void* data = obj->data;

	free(obj->key);
	obj->key = NULL;

	if (HT_UNDERFLOW(table))
		HashTableResize(table, HT_UNDERFLOW_NEW_SIZE(table));

	return data;
}

void* HashTableFind(HashTable* table,
	void* key,
	HT_INT key_size) {

	if (!table || !key || !key_size) return NULL;

	HTObject* obj = _HashTableFindObject(table, key, key_size);

	if (!obj || obj->key == NULL) return NULL;

	void* data = obj->data;

	return data;
}

HT_INT HashTableResize(HashTable* table, HT_INT new_size) {

	if (!table) return 0;

	new_size = is_prime(new_size) ? new_size : next_prime(new_size);

	if (new_size < MIN_TABLE_SIZE) return 0;

	/*
	* We can't get less space in the table than is necessary for storing data.
	* So, if $table->objects / $new_size >= $table->max_occupancy
	*		overflow will occur after the table is reduced!!!
	*/

	if ((float)table->objects / new_size >= table->max_occupancy) return 0;

	if (table->size == new_size) return 0;

	HTObject** old_tbl = table->table;

	HTObject** new_tbl = calloc(new_size, sizeof(HTObject*));
	if (!new_tbl) return 0;
	table->table = new_tbl;
	HT_INT old_size = table->size;
	table->size = new_size;
	table->deleted = 0;
	table->objects = 0;

	printf("RESIZE: NEW SIZE %d OLD SIZE %d\n", new_size, old_size);
	for (HT_INT cell = 0; cell < old_size; cell++) {
		if (!old_tbl[cell]) continue;
		if (!old_tbl[cell]->key) free(old_tbl[cell]);
		else {
			HashTableAdd(table, old_tbl[cell]->key, old_tbl[cell]->key_size, old_tbl[cell]->data);
			free(old_tbl[cell]->key);
		}
	}

	return new_size;
}

#define MAX_DEFAULT_HASH_DEPTH 19
HASH DEFAULT_FIRST_HASH(void* obj, HT_INT size) {
	HASH hash = 1, base = 0, tbase = 257;
	HT_INT depth = MIN(size, MAX_DEFAULT_HASH_DEPTH);

	for (int i = 0; i < depth; i++) {
		base = (i + 1) * tbase;
		hash += *((char*)obj) * base;
		((char*)obj)++;
	}

	return hash;
}


HASH DEFAULT_SECOND_HASH(void* obj, HT_INT size) {
	HASH hash = 1, base = 0, tbase = 19;
	HT_INT depth = MIN(size, MAX_DEFAULT_HASH_DEPTH);

	for (int i = 0; i < depth; i++) {
		base = (i + 1) * tbase;
		hash += *((char*)obj) * base;
		((char*)obj)++;
	}

	return hash % 2 ? hash + 1 : hash;
}


int main() {
	int n = 0, *t;
	HASH h;
	char key;
	HashTable* table = NewHashTable(0, 0, 0, 0);
	while (1) {
		scanf("%c %d", &key, &n);
		switch (key) {
		case 'a': 
			h = HashTableAdd(table, &n, sizeof(n), n);
			printf("%u\n", h);
			break;
		case 'r':
			t = HashTableRemove(table, &n, sizeof(n));
			printf("%d %d\n", t, t ? *t : t);
			break;
		case 'f':
			t = HashTableFind(table, &n, sizeof(n));
			printf("%d %d\n", t, t ? *t : t);
			break;
		}
	}
}

#undef HT_OVERFLOW
#undef HT_UNDERFLOW
#undef MIN
#undef MAX
#undef _CRT_SECURE_NO_WARNINGS
