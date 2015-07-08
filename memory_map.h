/*
 * memory_map.h
 *
 *  Created on: 8 may. 2015
 *      Author: enjschreuder
 */

#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

#include <stdbool.h>
#include <stdlib.h>

typedef size_t memory_map_iterator_t;

typedef struct
{
	unsigned char *	data;
	size_t			size;
	unsigned short	address;
} memory_map_region_t;

typedef struct
{
	memory_map_region_t **	region_list;
	size_t					size;
	size_t					length;
} memory_map_t;

memory_map_t * memory_map_create();
void memory_map_destroy(memory_map_t * memory_map);

memory_map_region_t * memory_map_region_create(size_t size);
void memory_map_region_destroy(memory_map_region_t * region);

size_t memory_map_get_length(memory_map_t * memory_map);

int memory_map_add_empty_region(memory_map_t * memory_map, unsigned short address, size_t size);
int memory_map_add_region(memory_map_t * memory_map, memory_map_region_t * region);
memory_map_region_t * memory_map_get_region(memory_map_t * memory_map, unsigned short address);

memory_map_iterator_t memory_map_get_iterator(memory_map_t * memory_map);
memory_map_region_t * memory_map_iterate(memory_map_t * memory_map, memory_map_iterator_t iterator);

#endif /* MEMORY_MAP_H_ */
