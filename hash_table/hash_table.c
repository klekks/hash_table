#include "hash_table.h"

#ifdef DEBUG

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#endif

#include <malloc.h>

#define MIN(A, B) A > B ? B : A
#define MAX(A, B) A < B ? B : A

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
	
	table->table = calloc(size, sizeof(HTObject));
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

int main() {
	int n = 0;
	while (1) {
		scanf("%d", &n);
		printf("%d\n", next_prime(n));
	}
}

#undef MIN
#undef MAX