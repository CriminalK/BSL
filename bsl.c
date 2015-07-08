/**
 * @file	bsl.c
 *
 * @date	11 feb. 2015
 * @author	enjschreuder
 * @brief	Source file for the Bootstrap Loader Protocol.
 *
 * @see		http://www.ti.com/lit/ug/slau319i/slau319i.pdf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bsl.h"
#include "serial.h"

#define BSL_TIMEOUT (1.0)
#define BSL_REQUEST_SIZE (10)
#define BSL_DATA_RESPONSE_SIZE (6)
#define BSL_ACK_RESPONSE_SIZE (1)
#define BSL_PASSWORD_SIZE (32)

static int bsl_write_request(bsl_object_t * object_p, unsigned char * data, size_t size);
static int bsl_read_data_response(bsl_object_t * object_p, unsigned char * data, size_t size);
static int bsl_read_ack_response(bsl_object_t * object_p);
static int bsl_send_synchronization_sequence(bsl_object_t * object_p);
static unsigned short bsl_calculate_checksum(const unsigned char * data, size_t size);

bsl_object_t * bsl_construct(int fd)
{
	bsl_object_t * object_p;

	// Allocate memory for the BSL object.
	object_p = malloc(sizeof(bsl_object_t));

	if (object_p == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the BSL object.\n");
	}
	else {
		// Store the file descriptor.
		object_p->fd = fd;
	}

	return object_p;
}

void bsl_destroy(bsl_object_t * object_p)
{
	// Free the object.
	free(object_p);
}

int bsl_initialize(bsl_object_t * object_p)
{
//	// Set ~RST to high and TEST to low.
//	serial_set_dtr(object_p->fd, true);
//	serial_set_rts(object_p->fd, false);

	// Set ~RST and TEST to low.
	serial_set_dtr(object_p->fd, false);
	serial_set_rts(object_p->fd, false);

	// Set TEST to high.
	serial_set_rts(object_p->fd, true);

	// Set TEST to low.
	serial_set_rts(object_p->fd, false);

	// Set TEST to high.
	serial_set_rts(object_p->fd, true);

	// Set ~RST to high.
	serial_set_dtr(object_p->fd, true);

	// Set TEST to low.
	serial_set_rts(object_p->fd, false);

	// Wait at least 100 microseconds.
	usleep(250000);

	return bsl_send_synchronization_sequence(object_p);
}

void bsl_terminate(bsl_object_t * object_p)
{
	// Set ~RST to low.
	serial_set_dtr(object_p->fd, false);

	// Set ~RST high.
	serial_set_dtr(object_p->fd, true);
}

int bsl_rx_data_block(bsl_object_t * object_p, unsigned short address, const unsigned char * data, size_t size)
{
	int error = 0;
	unsigned char * write_data;

	if (address % 2)
	{
		fprintf(stderr, "Register address should be a multiple of 2.\n");
		error = 1;
	}

	if ((size % 2) && (size < 250)) {
		fprintf(stderr, "Number of registers should be multiple of 1 and less than 250.\n");
		error = 1;
	}

	if (!error) {
		// Allocate memory for the request data.
		write_data = malloc(BSL_REQUEST_SIZE + size);
		if (write_data == NULL) {
			fprintf(stderr, "Failed to allocate memory for the request data.\n");
			error = 1;
		}
	}

	if (!error) {
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x12;
		write_data[2] = 4 + size;
		write_data[3] = write_data[2];
		write_data[4] = address % 256;
		write_data[5] = address / 256;
		write_data[6] = size;
		write_data[7] = 0x00;

		// Copy the data.
		memcpy(&(write_data[8]), data, size);

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	// Free the read_data (if possible).
	if (write_data != NULL) {
		free(write_data);
	}

	return error;
}

int bsl_rx_password(bsl_object_t * object_p, const unsigned char * password)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE + BSL_PASSWORD_SIZE];

	// Form the package.
	write_data[0] = 0x80;
	write_data[1] = 0x10;
	write_data[2] = 24;
	write_data[3] = write_data[2];
	write_data[4] = 0x00;
	write_data[5] = 0x00;
	write_data[6] = 0x00;
	write_data[7] = 0x00;

	// Copy the password.
	memcpy(&(write_data[8]), password, BSL_PASSWORD_SIZE);

	// Write the package.
	error = bsl_write_request(object_p, write_data, 8 + BSL_PASSWORD_SIZE);

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_erase_segment(bsl_object_t * object_p, unsigned short address)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];

	if (address % 2)
	{
		fprintf(stderr, "Register address should be a multiple of 2.\n");
		error = 1;
	}

	if (!error)
	{
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x16;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = address % 256;
		write_data[5] = address / 256;
		write_data[6] = 0x02;
		write_data[7] = 0xA5;

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_erase_main_info(bsl_object_t * object_p, unsigned short address)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];

	if (address % 2)
	{
		fprintf(stderr, "Register address should be a multiple of 2.\n");
		error = 1;
	}

	if (!error)
	{
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x16;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = address % 256;
		write_data[5] = address / 256;
		write_data[6] = 0x04;
		write_data[7] = 0xA5;

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_mass_erase(bsl_object_t * object_p)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];

	if (!error)
	{
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x18;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = 0x00;
		write_data[5] = 0x00;
		write_data[6] = 0x04;
		write_data[7] = 0xA5;

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_change_baudrate(bsl_object_t * object_p, bsl_baudrate_settings baudrate_settings)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];

	if (!error)
	{
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x20;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = baudrate_settings.clock_register_0;
		write_data[5] = baudrate_settings.clock_register_1;
		write_data[7] = 0x00;

		// Set the baud rate parameter.
		switch (baudrate_settings.bsl_baudrate)
		{
		case bsl_baudrate_9600:
			write_data[6] = 0x00;
			break;
		case bsl_baudrate_19200:
			write_data[6] = 0x01;
			break;
		case bsl_baudrate_38400:
			write_data[6] = 0x02;
			break;
		}

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_set_mem_offset(bsl_object_t * object_p, unsigned short offset)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];

	if (offset % 2)
	{
		fprintf(stderr, "Register offset should be a multiple of 2.\n");
		error = 1;
	}

	if (!error)
	{
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x21;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = 0x00;
		write_data[5] = 0x00;
		write_data[6] = offset % 256;
		write_data[7] = offset / 256;

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_load_pc(bsl_object_t * object_p, unsigned short address)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];

	if (address % 2)
	{
		fprintf(stderr, "Register address should be a multiple of 2.\n");
		error = 1;
	}

	if (!error)
	{
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x1A;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = address % 256;
		write_data[5] = address / 256;
		write_data[6] = 0x00;
		write_data[7] = 0x00;

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_ack_response(object_p);
	}

	return error;
}

int bsl_tx_data_block(bsl_object_t * object_p, unsigned short address, unsigned char * data, size_t size)
{
	int error = 0;
	unsigned char write_data[BSL_REQUEST_SIZE];
	unsigned char * read_data;

	if (address % 2)
	{
		fprintf(stderr, "Register address should be a multiple of 2.\n");
		error = 1;
	}

	if (size % 2)
	{
		fprintf(stderr, "Number of registers should be multiple of 2.\n");
		error = 1;
	}

	if (size > 250)
	{
		fprintf(stderr, "Number of registers should be less than 250.\n");
		error = 1;
	}

	if (size == 0)
	{
		fprintf(stderr, "Number of registers should be more than 0.\n");
		error = 1;
	}

	if (!error)
	{
		// Allocate memory for the response and the data.
		read_data = malloc(BSL_DATA_RESPONSE_SIZE + size);
		if (read_data == NULL) {
			fprintf(stderr, "Failed to allocate memory for the response data.\n");
			error = 1;
		}
	}

	if (!error) {
		// Form the package.
		write_data[0] = 0x80;
		write_data[1] = 0x14;
		write_data[2] = 4;
		write_data[3] = write_data[2];
		write_data[4] = address % 256;
		write_data[5] = address / 256;
		write_data[6] = (unsigned char) size;
		write_data[7] = 0x00;

		// Write the package.
		error = bsl_write_request(object_p, write_data, 8);
	}

	if (!error) {
		// Read the package.
		error = bsl_read_data_response(object_p, read_data, BSL_DATA_RESPONSE_SIZE + size);
	}

	if (!error) {
		// Copy the register data.
		memcpy(data, &read_data[4], size);
	}

	// Free the read_data (if possible).
	if (read_data != NULL) {
		free(read_data);
	}

	return error;
}

static int bsl_write_request(bsl_object_t * object_p, unsigned char * data, size_t size)
{
	unsigned short checksum;
	int error = 0;

	// Calculate the checksum.
	checksum = bsl_calculate_checksum(data, size);

	// Add the checksum to the data.
	data[size] = checksum % 256;
	size++;
	data[size] = checksum / 256;
	size++;

	// Send the synchronization sequence.
	error = bsl_send_synchronization_sequence(object_p);

	if (!error) {
		// Write the command.
		serial_write(object_p->fd, (char*) data, size);
	}

	return error;
}

static int bsl_read_ack_response(bsl_object_t * object_p)
{
	int error = 0;
	unsigned char data;
	size_t read_size = 0;

	// Read the package.
	read_size = serial_read(object_p->fd, (char *) &data, 1, BSL_TIMEOUT);
	if (read_size == 0) {
		fprintf(stderr, "Could not read the data response.\n");
		error = 1;
	}

	if (!error) {
		// Validate the package.
		if (data == 0xA0) {
			// Header incorrect.
			fprintf(stderr, "Received DATA_NACK.\n");
			error = 1;
		}
		else {
			if (data != 0x90) {
				// Header incorrect.
				fprintf(stderr, "Incorrect header, received header: 0x%2x.\n", data);
				error = 1;
			}
		}
	}

	return error;
}

static int bsl_read_data_response(bsl_object_t * object_p, unsigned char * data, size_t size)
{
	int error = 0;
	size_t read_size = 0;

	// Read the package.
	read_size = serial_read(object_p->fd, (char *) data, size, BSL_TIMEOUT);
	if (read_size == 0) {
		fprintf(stderr, "Could not read the data response.\n");
		error = 1;
	}

	if (!error) {
		// Validate the package.
		if (data[0] == 0xA0) {
			// Header incorrect.
			fprintf(stderr, "Received DATA_NACK.\n");
			error = 1;
		}
		else if (data[0] != 0x80) {
			// Header incorrect.
			fprintf(stderr, "Incorrect header, received header: 0x%2x.\n", data[0]);
			error = 1;
		}
		else if (data[2] != data[3]) {
			// Length fields unequal.
			fprintf(stderr, "Length fields do not match.\n");
			error = 1;
		}
		else if (data[2] != size - 6) {
			// Length field incorrect.
			fprintf(stderr, "Incorrect length.\n");
			error = 1;
		}
		else {
			// Check the checksum.
			unsigned short checksum;

			checksum = bsl_calculate_checksum(data, size - 2);
			if (((checksum % 256) != data[size - 2]) || ((checksum / 256) != data[size - 1])) {
				fprintf(stderr, "Incorrect checksum.\n");
				error = 1;
			}
		}
	}

	return error;
}

static int bsl_send_synchronization_sequence(bsl_object_t * object_p)
{
	int error = 0;
	unsigned char write_data = 0x80;
	unsigned char read_data;
	size_t read_size;

	// Send the 0x80 synchronisation character.
	serial_write(object_p->fd, (char *) &write_data, 1);

	// Read the 0x90 ACK character.
	read_size = serial_read(object_p->fd, (char *) &read_data, 1, BSL_TIMEOUT);

	if ((read_size != 1) || (read_data != 0x90))
	{
		fprintf(stderr, "Incorrect return for synchronisation sequence.\n");
		error = 1;
	}

	return error;
}

static unsigned short bsl_calculate_checksum(const unsigned char * data, size_t size)
{
	size_t i;
	unsigned short checksum;

	// Seed the checksum.
	checksum = data[0] + data[1] * 256;

	// XOR all unsigned short pairs.
	for (i = 2; i < size; i += 2)
	{
		checksum ^= data[i] + data[i + 1] * 256;
	}

	// Invert the checksum.
	checksum = ~checksum;

	return checksum;
}
