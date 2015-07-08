/*
 * device.h
 *
 *  Created on: 18 feb. 2015
 *      Author: enjschreuder
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdbool.h>
#include "bsl.h"

typedef struct
{
	bsl_object_t *	bsl_object_p;
	unsigned int	chip_id;
	unsigned int	bsl_version;
} device_object_t;

typedef struct
{
	bool main_memory;
	bool information_memory;
	bool segment_a;
} device_memory_sections_t;

device_object_t * device_construct(bsl_object_t * bsl_object_p);
void device_destroy(device_object_t * device_object_p);

int device_initialize(device_object_t * device_object_p, const unsigned char * password);
void device_terminate(device_object_t * object_p);

unsigned int device_get_chip_id(device_object_t * object_p);
unsigned int device_get_bsl_version(device_object_t * object_p);

int device_read_memory(device_object_t * object_p, unsigned short address, unsigned char * data, size_t length);
int device_write_memory(device_object_t *object_p, unsigned short address, const unsigned char * data, size_t length);
int device_erase_memory(device_object_t *object_p, device_memory_sections_t memory_sections);

#endif /* DEVICE_H_ */
