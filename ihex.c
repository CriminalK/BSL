/*
 * ihex.c
 *
 *  Created on: 8 may. 2015
 *      Author: enjschreuder
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ihex.h"

#define IHEX_DATA_RECORD_TYPE				(0x00)
#define IHEX_EOF_RECORD_TYPE				(0x01)
#define IHEX_EXTENDED_SEGMENT_ADDRESS_TYPE	(0x02)
#define IHEX_START_SEGMENT_ADDRESS_TYPE		(0x03)
#define IHEX_EXTENDED_LINEAR_ADDRESS_TYPE	(0x04)
#define IHEX_START_LINEAR_ADDRESS_TYPE		(0x05)

#define IHEX_DEFAULT_SIZE					(5)

static int ihex_from_file(ihex_t * ihex, FILE * file);
void ihex_to_file(const ihex_t * ihex, FILE * file);

static int ihex_check_file(FILE * file, size_t * size_p);
static int ihex_verify_checksum(const char * line, size_t reclen);
static int ihex_get_record_information(const char * input_string, unsigned char * reclen_p, unsigned short * load_offset_p, unsigned char * rectyp_p);
static int ihex_get_data(const char * input_string, unsigned char * data, size_t data_size);

ihex_t * ihex_create()
{
	ihex_t * ihex = NULL;

	// Allocate memory for the ihex  object.
	ihex = malloc(sizeof(ihex_t));

	if (ihex == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the ihex object.\n");
	}
	else {
		// Set initial size and length
		ihex->records = NULL;
		ihex->size = 0;
	}

	return ihex;
}

void ihex_destroy(ihex_t * ihex)
{
	size_t i;

	if (ihex != NULL)
	{
		if (ihex->records != NULL) {
			for (i = 0; i < ihex->size; i++) {
				if (ihex->records[i].data != NULL) {
					free(ihex->records[i].data);
				}
			}
			free(ihex->records);
		}
		free(ihex);
	}
}

int ihex_reset(ihex_t * ihex, size_t size)
{
	int error = 0;
	size_t i;

	if (ihex->records != NULL)
	{
		for (i = 0; i < ihex->size; i++) {
			if (ihex->records[i].data != NULL) {
				free(ihex->records[i].data);
			}
		}
		free(ihex->records);
	}

	// Allocate memory for the ihex object.
	ihex->records = malloc(sizeof(ihex_record_t) * size);

	if (ihex->records == NULL) {
		// Could not allocate memory.
		fprintf(stderr, "Failed to allocate memory for the ihex object.\n");
		error = 1;
	}
	else {
		ihex->size = size;
		// Set initial parameters
		for (i = 0; i < ihex->size; i++) {
			ihex->records[i].data = NULL;
			ihex->records[i].data_size = 0;
			ihex->records[i].load_offset = 0;
			ihex->records[i].rectype = IHEX_RECTYPE_DATA_RECORD;
		}
	}

	return error;
}

int ihex_read_file(ihex_t * ihex, const char * filename)
{
	int		error = 0;
	FILE *	file;
	size_t	size = 0;

	// Open the file.
	file = fopen(filename, "r");
	if (file == NULL) {
		// Could not open file.
		fprintf(stderr, "Failed to open file %s.\n", filename);
		error = 1;
	}

	if (!error) {
		// Check the file.
		error = ihex_check_file(file, &size);
	}

	if (!error) {
		// Set the file pointer back to the start of the file.
		rewind(file);

		// Set the length of the ihex object.
		error = ihex_reset(ihex, size);
	}

	if (!error) {
		// Parse the file to a memory map.
		error = ihex_from_file(ihex, file);
	}

	if (file != NULL) {
		// Close the file if necessary.
		fclose(file);
	}

	return error;
}

int ihex_write_file(const ihex_t * ihex, const char * filename)
{
	int		error = 0;
	FILE *	file;

	// Open the file.
	file = fopen(filename, "w");
	if (file == NULL) {
		// Could not open file.
		fprintf(stderr, "Failed to open file %s.\n", filename);
		error = 1;
	}

	if (!error) {
		// Parse ihex to a file.
		ihex_to_file(ihex, file);
	}

	if (file != NULL) {
		// Close the file if necessary.
		fclose(file);
	}

	return error;
}

void ihex_print(ihex_t * ihex)
{
	size_t record;
	size_t i;

	unsigned int address = 0;
	unsigned short offset;

	for (record = 0; (record < ihex->size); record++) {
		// Get the record type.
		switch(ihex->records[record].rectype) {
		case IHEX_RECTYPE_DATA_RECORD:
			printf("%08x: ", address + ihex->records[record].load_offset);
			for (i = 0; (i < ihex->records[record].data_size); i++) {
				printf("%02x ", ihex->records[record].data[i]);
			}
			printf("\n");
			break;
		case IHEX_RECTYPE_EXTENDED_SEGMENT_ADDRESS_TYPE:
			offset = ihex->records[record].data[0] * 256 + ihex->records[record].data[1];
			address &= ~0xFFFF;
			address += offset;
			break;
		case IHEX_RECTYPE_EXTENDED_LINEAR_ADDRESS_TYPE:
			offset = ihex->records[record].data[0] * 256 + ihex->records[record].data[1];
			address &= ~0xFFFF0000;
			address += offset << 16;
			break;
		case IHEX_RECTYPE_START_SEGMENT_ADDRESS_TYPE:
			break;
		case IHEX_RECTYPE_START_LINEAR_ADDRESS_TYPE:
			break;
		}
	}
}

static int ihex_from_file(ihex_t * ihex, FILE * file)
{
	int		error = 0;
	char	input_string[1000];
	size_t	record;

	// Scroll through the file to read the current address and data size.
	for (record = 0; (record < ihex->size) && !error; record++)
	{
		unsigned char	reclen;
		unsigned short	load_offset;
		unsigned char	rectyp;

		if (!fgets(input_string, sizeof(input_string) - 1, file)) {
			fprintf(stderr, "Failed to line from file.\n");
			error = 1;
		}

		if (!error) {
			// Retrieve record information.
			error = ihex_get_record_information(input_string, &reclen, &load_offset, &rectyp);
		}

		if (!error) {
			ihex->records[record].data_size = reclen;
			ihex->records[record].load_offset = load_offset;

			switch (rectyp) {
			case IHEX_DATA_RECORD_TYPE:
				ihex->records[record].rectype = IHEX_RECTYPE_DATA_RECORD;
				break;
			case IHEX_EXTENDED_SEGMENT_ADDRESS_TYPE:
				ihex->records[record].rectype = IHEX_RECTYPE_EXTENDED_SEGMENT_ADDRESS_TYPE;
				break;
			case IHEX_START_SEGMENT_ADDRESS_TYPE:
				ihex->records[record].rectype = IHEX_RECTYPE_START_SEGMENT_ADDRESS_TYPE;
				break;
			case IHEX_EXTENDED_LINEAR_ADDRESS_TYPE:
				ihex->records[record].rectype = IHEX_RECTYPE_EXTENDED_LINEAR_ADDRESS_TYPE;
				break;
			case IHEX_START_LINEAR_ADDRESS_TYPE:
				ihex->records[record].rectype = IHEX_RECTYPE_START_LINEAR_ADDRESS_TYPE;
				break;
			default:
				fprintf(stderr, "Unknown record detected.\n");
				error = 1;
				break;
			}
		}

		if (!error) {
			ihex->records[record].data = malloc(ihex->records[record].data_size);

			if (ihex->records[record].data == NULL) {
				// Could not allocate memory.
				fprintf(stderr, "Failed to allocate memory for the data in a record.\n");
				error = 1;
			}
		}

		if (!error) {
			error = ihex_get_data(input_string, ihex->records[record].data, ihex->records[record].data_size);
		}
	}

	return error;
}

void ihex_to_file(const ihex_t * ihex, FILE * file)
{
	size_t record;
	size_t i;

	for (record = 0; (record < ihex->size); record++) {
		unsigned char record_type;
		unsigned char checksum;

		// Get the record type.
		switch(ihex->records[record].rectype) {
		case IHEX_RECTYPE_DATA_RECORD:
			record_type = IHEX_DATA_RECORD_TYPE;
			break;
		case IHEX_RECTYPE_EXTENDED_SEGMENT_ADDRESS_TYPE:
			record_type = IHEX_EXTENDED_SEGMENT_ADDRESS_TYPE;
			break;
		case IHEX_RECTYPE_START_SEGMENT_ADDRESS_TYPE:
			record_type = IHEX_START_SEGMENT_ADDRESS_TYPE;
			break;
		case IHEX_RECTYPE_EXTENDED_LINEAR_ADDRESS_TYPE:
			record_type = IHEX_EXTENDED_LINEAR_ADDRESS_TYPE;
			break;
		case IHEX_RECTYPE_START_LINEAR_ADDRESS_TYPE:
			record_type = IHEX_START_LINEAR_ADDRESS_TYPE;
			break;
		}

		fprintf(file, ":%02x%04x%02x",
				(unsigned char) ihex->records[record].data_size, ihex->records[record].load_offset, record_type);

		checksum =	ihex->records[record].data_size +
					ihex->records[record].load_offset / 256 +
					ihex->records[record].load_offset % 256 +
					record_type;

		for (i = 0; (i < ihex->records[record].data_size); i++) {
			fprintf(file, "%02x", ihex->records[record].data[i]);
			checksum += ihex->records[record].data[i];
		}

		checksum = -checksum;

		fprintf(file, "%02x\n", checksum);
	}

	// Write the EOF.
	fprintf(file, ":00000001FF\n");
}


static int ihex_check_file(FILE * file, size_t * size_p)
{
	int error = 0;
	bool completed = false;
	size_t line = 1;
	char input_string[1000];

	*size_p = 0;

	while (fgets(input_string, sizeof(input_string) - 1, file) && !completed && !error) {
		unsigned char	reclen;
		unsigned short	load_offset;
		unsigned char	rectyp;

		if (strlen(input_string) >= (sizeof(input_string) - 1)) {
			fprintf(stderr, "Error on line %u; line too long.\n", (unsigned int) line);
			error = 1;
		}

		if (!error && (input_string[0] != ':'))
		{
			// Check if the string contains a record.
			fprintf(stderr, "Error on line %u; record does not start with :.\n", (unsigned int) line);
			error = 1;
		}

		if (!error) {
			error = ihex_get_record_information(input_string, &reclen, &load_offset, &rectyp);
			if (error) {
				fprintf(stderr, "Error on line %u; cannot read record fields.\n", (unsigned int) line);
				error = 1;
			}
		}
		if (!error) {
			// Check the checksum.
			error = ihex_verify_checksum(input_string, reclen);
			if (error) {
				fprintf(stderr, "Checksum for line %u incorrect.\n", (unsigned int) line);
			}
			else {
				switch (rectyp) {
				case IHEX_DATA_RECORD_TYPE:
				case IHEX_EXTENDED_SEGMENT_ADDRESS_TYPE:
				case IHEX_START_SEGMENT_ADDRESS_TYPE:
				case IHEX_EXTENDED_LINEAR_ADDRESS_TYPE:
				case IHEX_START_LINEAR_ADDRESS_TYPE:
					(*size_p)++;
					break;
				case IHEX_EOF_RECORD_TYPE:
					completed = true;
					break;
				default:
					fprintf(stderr, "Unknown record detected.\n");
					error = 1;
					break;
				}
			}
		}

		// Update the line number.
		line++;
	}

	if (!completed) {
		fprintf(stderr, "No EOF record present.\n");
		error = 1;
	}

	return error;
}

static int ihex_verify_checksum(const char * line, size_t reclen)
{
	int error = 0;
	size_t i;
	unsigned char sum = 0;

	// For a line, calculate the checksum
	for (i = 1; (i < (2 + 4 + 2 + reclen * 2 + 2)) && !error; i += 2) {
		unsigned int temp_data;

		if(!sscanf(&(line[i]), "%02x", &temp_data)) {
			error = 1;
		}
		else {
			sum += (unsigned char) temp_data;
		}
	}

	// Check if the sum is zero.
	if (sum != 0) {
		error = 1;
	}

	return error;
}

static int ihex_get_record_information(const char * input_string, unsigned char * reclen_p, unsigned short * load_offset_p, unsigned char * rectyp_p)
{
	int error = 0;
	unsigned int reclen;
	unsigned int load_offset;
	unsigned int rectyp;

	if (!sscanf(input_string, ":%02x%04x%02x", &reclen, &load_offset, &rectyp)) {
		error = 1;
	}
	else {
		*reclen_p = (unsigned char) reclen;
		*load_offset_p = (unsigned short) load_offset;
		*rectyp_p = (unsigned char) rectyp;
	}

	return error;
}

static int ihex_get_data(const char * input_string, unsigned char * data, size_t data_size)
{
	int error = 0;
	size_t i;
	unsigned int temp_data;

	for (i = 0; i < data_size; i++)
	{
		if(!sscanf(&(input_string[9 + i * 2]), "%02x", &temp_data)) {
			error = 1;
		}
		else {
			data[i] = (unsigned char) temp_data;
		}
	}

	return error;
}
