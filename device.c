/*
 * device.c
 *
 *  Created on: 18 feb. 2015
 *      Author: enjschreuder
 */

#include <stdlib.h>
#include <stdio.h>

#include "serial.h"
#include "device.h"
#include "bsl.h"

#define DEVICE_MAIN_MEMORY_ADDRESS			(0xFFFE)
#define DEVICE_INFORMATION_MEMORY_ADDRESS	(0x1000)
#define DEVICE_SEGMENT_A_ADDRESS			(0x10C0)
#define DEVICE_SEGMENT_B_ADDRESS			(0x1080)
#define DEVICE_SEGMENT_C_ADDRESS			(0x1040)
#define DEVICE_SEGMENT_D_ADDRESS			(0x1000)

#define DEVICE_CHIP_ID_ADDRESS				(0x0FF0)
#define DEVICE_BSL_VERSION_ADDRESS			(0x0FFA)

device_object_t * device_construct(bsl_object_t * bsl_object_p)
{
	device_object_t * object_p;

	// Allocate memory for the BSL object.
	object_p = malloc(sizeof(device_object_t));

	if (object_p == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the device object.\n");
	}
	else
	{
		// Store the bsl object pointer.
		object_p->bsl_object_p = bsl_object_p;
		object_p->chip_id = 0;
		object_p->bsl_version = 0;
	}

	return object_p;
}

void device_destroy(device_object_t * object_p)
{
	free(object_p);
}

int device_initialize(device_object_t * object_p, const unsigned char * password)
{
	int error = 0;

	if (!error) {
		// Start the bsl.
		error = bsl_initialize(object_p->bsl_object_p);
	}

	if (!error && password) {
		// Send the password.
		error = bsl_rx_password(object_p->bsl_object_p, password);
		if (error)
		{
			fprintf(stderr, "Sending password failed, device is possibly mass erased.\n");
		}
	}

	if (!error) {
		bsl_baudrate_settings baudrate_settings = {0x80, 0x8C, bsl_baudrate_38400};

		// Increase the baudrate on the device.
		error = bsl_change_baudrate(object_p->bsl_object_p, baudrate_settings);
		if (error)
		{
			fprintf(stderr, "Changing the baudrate to 38400 baud failed.\n");
		}
	}

	if (!error) {
		// Increase the serial baudrate.
		serial_change_baudrate(object_p->bsl_object_p->fd, baudrate_38400);
	}

	if (!error) {
		// Read the Chip ID.
		unsigned char chip_id_data[2];
		error = bsl_tx_data_block(object_p->bsl_object_p, DEVICE_CHIP_ID_ADDRESS, chip_id_data, 2);
		object_p->chip_id = chip_id_data[0] * 256 + chip_id_data[1];
	}

	if (!error) {
		// Read the BSL version.
		unsigned char bsl_version_data[2];
		error = bsl_tx_data_block(object_p->bsl_object_p, DEVICE_BSL_VERSION_ADDRESS, bsl_version_data, 2);
		object_p->bsl_version = bsl_version_data[0] * 256 + bsl_version_data[1];
	}

	return error;
}

void device_terminate(device_object_t * object_p)
{
	// Stop the bsl.
	bsl_terminate(object_p->bsl_object_p);
}

unsigned int device_get_chip_id(device_object_t * object_p)
{
	return object_p->chip_id;
}

unsigned int device_get_bsl_version(device_object_t * object_p)
{
	return object_p->bsl_version;
}

int device_read_memory(device_object_t * object_p, unsigned short address, unsigned char * data, size_t length)
{
	int error = 0;
	size_t i;

	// Maximum of 250 bytes can be read at a time.
	for (i = 0; (i < length) && !error; i += 250) {

		// Set the maximum size
		size_t read_size = length - i;
		if (read_size > 250) {
			read_size = 250;
		}

		// Retrieve the data.
		error = bsl_tx_data_block(object_p->bsl_object_p, address + i, &(data[i]), read_size);
	}

	return error;
}

int device_write_memory(device_object_t *object_p, unsigned short address, const unsigned char * data, size_t length)
{
	int error = 0;
	size_t i;

	// Maximum of 250 bytes can be read at a time.
	for (i = 0; (i < length) && !error; i += 250) {

		// Set the maximum size
		size_t write_size = length - i;
		if (write_size > 250) {
			write_size = 250;
		}

		// Retrieve the data.
		error = bsl_rx_data_block(object_p->bsl_object_p, address + i, &(data[i]), write_size);
	}

	return error;
}

int device_erase_memory(device_object_t *object_p, device_memory_sections_t memory_sections)
{
	int error = 0;

	if (memory_sections.main_memory && memory_sections.information_memory && memory_sections.segment_a) {
		// In case all memory sections can be erased, just do a mass erase.
		error = bsl_mass_erase(object_p->bsl_object_p);
	}
	else
	{
		if (memory_sections.main_memory) {
			error = bsl_erase_main_info(object_p->bsl_object_p, DEVICE_MAIN_MEMORY_ADDRESS);
		}
		if (memory_sections.information_memory)
		{
			if (memory_sections.segment_a) {
				// In case segment A can be erased, do a full wipe of the information memory.
				error = bsl_erase_main_info(object_p->bsl_object_p, DEVICE_INFORMATION_MEMORY_ADDRESS);
			}
			else {
				// Otherwise just erase the other segments in the information memory.
				error = bsl_erase_segment(object_p->bsl_object_p, DEVICE_SEGMENT_B_ADDRESS);
				error = bsl_erase_segment(object_p->bsl_object_p, DEVICE_SEGMENT_C_ADDRESS);
				error = bsl_erase_segment(object_p->bsl_object_p, DEVICE_SEGMENT_D_ADDRESS);
			}
		}
	}

	return error;
}
