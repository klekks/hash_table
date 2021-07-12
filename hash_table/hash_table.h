#pragma once

#define HASH unsigned long long
#define HT_INT unsigned
#define DEBUG 

#define DEFAULT_OCCUPANCY 0.5f
#define OCCUPANCY_MIN 0.25f
#define OCCUPANCY_MAX 0.75f

#define MIN_TABLE_SIZE 17

typedef struct {
	void* data;
	HT_INT data_size;
	HASH first_hash;
	HASH second_hash;
} HTObject;

typedef struct {
	HTObject* table;
	HT_INT size;    // the number of $table cells 
	HT_INT deleted; // the number of deleted objects
	HT_INT objects; // the number of valid objects
	HASH (*first_hash_function) (void*, HT_INT);
	HASH (*second_hash_function)(void*, HT_INT);
	float max_occupancy; // if (float) 
						 // ($deleted + $objects) / $size > $max_occupancy 
						 // then resize
						 // OCCUPANCY_MIN <= &max_occupancy <= OCCUPANCY_MAX
} HashTable;

HashTable* NewHashTable(HT_INT size,
					    HASH(*first_hash_function) (void*),
					    HASH(*second_hash_function)(void*),
					    float max_occupancy);

HT_INT HashTableAdd(HashTable *table, 
					void* key, 
					HT_INT key_size, 
					void* data, 
					HT_INT data_size);

HT_INT HashTableFind(HashTable *table, 
					 void* key, 
					 HT_INT key_size);

HT_INT HashTableRemove(HashTable *table,
					   void* key,
					   HT_INT key_size);

HT_INT HashTableResize(HashTable* table);

HASH DEFAULT_FIRST_HASH(void* obj, HT_INT size);
HASH DEFAULT_SECOND_HASH(void* obj, HT_INT size);