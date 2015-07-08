/**
 * @file	serial.h
 *
 * @date	29 jan. 2015
 * @author	enjschreuder
 * @brief	Header file for serial library.
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @addtogroup serial
 * @{
 */

/**
 * @brief Available baud rates.
 */
typedef enum
{
	baudrate_0,		/**< 0 baud.		*/
	baudrate_50,	/**< 50 baud.		*/
	baudrate_75,	/**< 75 baud.		*/
	baudrate_110,	/**< 110 baud.		*/
	baudrate_134,	/**< 134 baud.		*/
	baudrate_150,	/**< 150 baud.		*/
	baudrate_200,	/**< 200 baud.		*/
	baudrate_300,	/**< 300 baud.		*/
	baudrate_600,	/**< 600 baud.		*/
	baudrate_1200,	/**< 1200 baud.		*/
	baudrate_1800,	/**< 1800 baud.		*/
	baudrate_2400,	/**< 2400 baud.		*/
	baudrate_4800,	/**< 4800 baud.		*/
	baudrate_9600,	/**< 9600 baud.		*/
	baudrate_19200,	/**< 19200 baud.	*/
	baudrate_38400,	/**< 38400 baud.	*/
	baudrate_57600,	/**< 57600 baud.	*/
	baudrate_115200	/**< 115200 baud.	 */
} serial_baudrate;

/**
 * @brief Parity setting.
 */
typedef enum
{
	none,	/**< No parity. 	*/
	odd,	/**< Odd parity. 	*/
	even	/**< Even parity. 	*/
} serial_parity;

/**
 * @brief Stopbit setting.
 */
typedef enum
{
	stopbits_1,	/**< One stopbit.	*/
	stopbits_2	/**< Two stopbits.	*/
} serial_stopbits;

/**
 * @brief Databit setting.
 */
typedef enum
{
	databits_5,	/**< Five databits.		*/
	databits_6,	/**< Six databits.		*/
	databits_7,	/**< Seven databits.	*/
	databits_8,	/**< Eight databits.	*/
} serial_databits;

/**
 * @brief Settings for opening a serial port.
 */
typedef struct
{
	serial_baudrate		baudrate;		/**< The serial baudrate.										*/
	serial_parity		parity;			/**< The parity setting.										*/
	serial_stopbits		stopbits;		/**< The number of stopbits. 									*/
	serial_databits		databits;		/**< The number of databits. 									*/
	bool				flow_control;	/**< Hardware flow control: TRUE = enabled, FALSE = disabled.	*/
} serial_settings_t;

int serial_open(const char* serial_port, serial_settings_t settings);
void serial_close(int fd);
int serial_write(int fd, const char* data, size_t size);
int serial_read(int fd, char* data, size_t size, double timeout);
void serial_set_rts(int fd, bool enabled);
void serial_set_dtr(int fd, bool enabled);
bool serial_get_cts(int fd);
bool serial_get_dcd(int fd);
bool serial_get_dsr(int fd);
void serial_change_baudrate(int fd, serial_baudrate baudrate);

/**
 * @}
 */

#endif /* SERIAL_H_ */
