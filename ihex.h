/*
 * ihex.h
 *
 *  Created on: 8 may. 2015
 *      Author: enjschreuder
 */

#include <stdlib.h>

typedef enum
{
	IHEX_RECTYPE_DATA_RECORD,
	IHEX_RECTYPE_EXTENDED_SEGMENT_ADDRESS_TYPE,
	IHEX_RECTYPE_START_SEGMENT_ADDRESS_TYPE,
	IHEX_RECTYPE_EXTENDED_LINEAR_ADDRESS_TYPE,
	IHEX_RECTYPE_START_LINEAR_ADDRESS_TYPE
} ihex_rectype_enum;

typedef struct
{
	unsigned short		load_offset;
	ihex_rectype_enum	rectype;
	size_t				data_size;
	unsigned char *		data;
} ihex_record_t;

typedef struct
{
	size_t				size;
	ihex_record_t *		records;
} ihex_t;

ihex_t * ihex_create();
void ihex_destroy(ihex_t * ihex);

int ihex_reset(ihex_t * ihex, size_t size);

int ihex_read_file(ihex_t * ihex, const char * filename);
int ihex_write_file(const ihex_t * ihex, const char * filename);

void ihex_print(ihex_t * ihex);
