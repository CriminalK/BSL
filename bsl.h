/**
 * @file	bsl.c
 *
 * @date	11 feb. 2015
 * @author	enjschreuder
 * @brief	Header file for the Bootstrap Loader Protocol.
 */

#ifndef BSL_H_
#define BSL_H_

#include <stddef.h>

typedef struct
{
	int fd;

} bsl_object_t;

typedef struct
{
	unsigned char	clock_register_0;
	unsigned char	clock_register_1;
	enum
	{
		bsl_baudrate_9600,	/**< 9600 baud.		*/
		bsl_baudrate_19200,	/**< 19200 baud.	*/
		bsl_baudrate_38400,	/**< 38400 baud.	*/
	}				bsl_baudrate;
} bsl_baudrate_settings;

bsl_object_t * bsl_construct(int fd);
void bsl_destroy(bsl_object_t * object_p);

int bsl_initialize(bsl_object_t * object_p);
void bsl_terminate(bsl_object_t * object_p);

int bsl_rx_data_block(bsl_object_t * object_p, unsigned short address, const unsigned char * data, size_t size);
int bsl_rx_password(bsl_object_t * object_p, const unsigned char * password);
int bsl_erase_segment(bsl_object_t * object_p, unsigned short address);
int bsl_erase_main_info(bsl_object_t * object_p, unsigned short address);
int bsl_mass_erase(bsl_object_t * object_p);
int bsl_change_baudrate(bsl_object_t * object_p, bsl_baudrate_settings baudrate_settings);
int bsl_set_mem_offset(bsl_object_t * object_p, unsigned short offset);
int bsl_load_pc(bsl_object_t * object_p, unsigned short address);
int bsl_tx_data_block(bsl_object_t * object_p, unsigned short address, unsigned char * data, size_t size);

#endif /* BSL_H_ */
