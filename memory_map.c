/*
 * memory_map.c
 *
 *  Created on: 8 may. 2015
 *      Author: enjschreuder
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory_map.h"

#define MEMORY_MAP_DEFAULT_SIZE	(5)

memory_map_t * memory_map_create()
{
	memory_map_t * memory_map = NULL;
	memory_map_region_t ** region_list = NULL;

	// Allocate memory for the region array in the memory map object.
	region_list = malloc(sizeof(memory_map_region_t *) * MEMORY_MAP_DEFAULT_SIZE);

	if (region_list == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the region array in the memory map object.\n");
	}
	else {
		// Allocate memory for the memory map object.
		memory_map = malloc(sizeof(memory_map_t));
	}

	if (memory_map == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the memory map object.\n");

		// Free already allocated memory.
		if (region_list != NULL) {
			free(region_list);
		}
	}
	else {
		// Set initial size and length
		memory_map->length = 0;
		memory_map->size = MEMORY_MAP_DEFAULT_SIZE;
		memory_map->region_list = region_list;
	}

	return memory_map;
}

void memory_map_destroy(memory_map_t * memory_map)
{
	size_t i;

	// For all memory regions, free the data memory.
	for (i = 0; i < memory_map->length; i++) {
		memory_map_region_destroy(memory_map->region_list[i]);
	}

	// Free the memory region array.
	free(memory_map->region_list);

	// Free the memory map object.
	free(memory_map);
}

memory_map_region_t * memory_map_region_create(size_t size)
{
	memory_map_region_t * region = NULL;
	unsigned char * data = NULL;

	// Allocate memory for the region array in the memory map object.
	data = malloc(size);

	if (data == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the memory region data.\n");
	}
	else {
		// Allocate memory for the memory map region data.
		region = malloc(sizeof(memory_map_region_t));
	}

	if (region == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the memory map region object.\n");

		// Free already allocated memory.
		if (data != NULL) {
			free(data);
		}
	}
	else {
		// Set object data.
		region->address = 0;
		region->data = data;
		region->size = size;
	}

	return region;
}

void memory_map_region_destroy(memory_map_region_t * region)
{
	free(region->data);
}

size_t memory_map_get_length(memory_map_t * memory_map)
{
	return memory_map->length;
}

int memory_map_add_empty_region(memory_map_t * memory_map, unsigned short address, size_t size)
{
	int error = 0;

	// Create new memory region at address.
	memory_map_region_t * region = memory_map_region_create(size);

	if (region == NULL)
	{
		fprintf(stderr, "Failed to create a new memory map region object.\n");
		error = 1;
	}
	else
	{
		region->address = address;
	}

	// Add the memory region to the memory map.
	if (!error)
	{
		error = memory_map_add_region(memory_map, region);
	}

	// If an error occurred, destroy the memory region again.
	if (error)
	{
		if (region != NULL)
		{
			memory_map_region_destroy(region);
		}
	}

	return error;
}

int memory_map_add_region(memory_map_t * memory_map, memory_map_region_t * region)
{
	int error = 0;
	size_t i;

	// Check if this memory region is already present.
	for (i = 0; (i < memory_map->length) && !error; i++) {
		unsigned short current_address_start = memory_map->region_list[i]->address;
		unsigned short current_address_end = current_address_start + memory_map->region_list[i]->size;
		unsigned short new_address_start = region->address;
		unsigned short new_address_end = new_address_start + region->size;

		if (((new_address_start >= current_address_start) && (current_address_start <= current_address_end)) ||
			((new_address_end >= current_address_start) && (new_address_end <= current_address_end)))
		{
			fprintf(stderr, "Region overlaps with those existing in the memory map.\n");
			error = 1;
		}
	}

	if (!error) {
		// Check if memory can still be allocated, if not add more.
		if (memory_map->length >= memory_map->size) {
			memory_map_region_t ** region_list = realloc(	memory_map->region_list,
															sizeof(memory_map_region_t *) * (MEMORY_MAP_DEFAULT_SIZE + memory_map->size));

			if (region_list == NULL) {
				fprintf(stderr, "Failed to add memory for the region pointers array.\n");
				error = 1;
			}
			else {
				memory_map->region_list = region_list;
				memory_map->size += MEMORY_MAP_DEFAULT_SIZE;
			}
		}
	}

	if (!error) {
		// Copy data in a new region.
		memory_map->region_list[memory_map->length] = region;

		memory_map->length++;
	}

	return error;
}

memory_map_region_t * memory_map_get_region(memory_map_t * memory_map, unsigned short address)
{
	memory_map_region_t * region = NULL;
	size_t i;

	// Loop over all memory entries to get the correct region.
	for (i = 0; i < memory_map->length; i++) {
		if (memory_map->region_list[i]->address == address) {
			region = memory_map->region_list[i];
			break;
		}
	}

	return region;
}


memory_map_iterator_t memory_map_get_iterator(memory_map_t * object_p)
{
	return 0;
}

memory_map_region_t * memory_map_iterate(memory_map_t * memory_map, memory_map_iterator_t iterator)
{
	memory_map_region_t * region = NULL;

	if (iterator < memory_map->length) {
		region = memory_map->region_list[iterator];
		iterator++;
	}

	return region;
}
