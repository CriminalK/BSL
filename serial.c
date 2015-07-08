/**
 * @file	serial.c
 *
 * @date	29 jan. 2015
 * @author	enjschreuder
 * @brief	Source file for serial library.
 *
 * @see		https://www.cmrr.umn.edu/~strupp/serial.html
 */

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "serial.h"

/**
 * @defgroup serial Serial Port
 * @brief Library functions for utilizing the serial port.
 * @{
 */

static speed_t _serial_get_baudrate(serial_baudrate baudrate);

/**
 * @brief	Opens the serial port.
 * @param	serial_port		Name of the serial port.
 * @param	settings		Settings for this serial port.
 * @return	File descriptor for the serial port.
 */
int serial_open(const char * serial_port, serial_settings_t settings)
{
	struct termios options;
	speed_t baudrate;
	int fd;

	// Open the serial port.
	fd = open(serial_port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd == -1) {
		// Could not open the port.
		fprintf(stderr, "Could not open serial port %s: %s\n", serial_port, strerror(errno));
	}
	else {
		// Get the current options.
		tcgetattr(fd, &options);

		// Set the baud rate.
		baudrate = _serial_get_baudrate(settings.baudrate);
		cfsetispeed(&options, baudrate);
		cfsetospeed(&options, baudrate);

		options.c_oflag = 0;

		// Enable the receiver and local mode.
		options.c_cflag |= (CLOCAL | CREAD);

		// Setup the parity
		switch (settings.parity)
		{
		case none:
			options.c_cflag &= ~PARENB;
			break;
		case odd:
			options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;
			break;
		case even:
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			break;
		}

		// Setup the databits.
		options.c_cflag &= ~CSIZE;
		switch(settings.databits)
		{
		case databits_5:
			options.c_cflag |= CS5;
			break;
		case databits_6:
			options.c_cflag |= CS6;
			break;
		case databits_7:
			options.c_cflag |= CS7;
			break;
		case databits_8:
			options.c_cflag |= CS8;
			break;
		}

		// Setup the stopbits.
		switch(settings.stopbits)
		{
		case stopbits_1:
			options.c_cflag &= ~CSTOPB;
			break;
		case stopbits_2:
			options.c_cflag |= ~CSTOPB;
			break;
		}

		// Setup the flowcontrol.
		if (settings.flow_control) {
			options.c_cflag |= CRTSCTS;
		}
		else {
			options.c_cflag &= ~CRTSCTS;
		}

		// Write the options to the port.
		tcsetattr(fd, TCSANOW, &options);
	}

	return fd;
}

/**
 * @brief	Closes the serial port.
 * @param	fd		File descriptor for the serial port.
 * @return	None.
 */
void serial_close(int fd)
{
	close(fd);
}

/**
 * @brief	Write to the serial port.
 * @param	fd				File descriptor for the serial port.
 * @param	data			Data to write.
 * @param	size			Amount of data to write.
 * @return	Amount of data written.
 */
int serial_write(int fd, const char* data, size_t size)
{
	int written_size;

	size_t i;
	printf("Writing: ");
	for (i = 0; i < size; i++)
	{
		printf("0x%02x ", (unsigned char) data[i]);
	}
	printf("\n");

	// Write the data over the serial port.
	written_size = write(fd, data, size);
	if (written_size != size) {
		// Could not send the data to the port.
		fprintf(stderr, "Could not send serial data serial port: %s\n", strerror(errno));
	}

	return written_size;
}

/**
 * @brief	Read the serial port.
 * @param	fd				File descriptor for the serial port.
 * @param	data			Buffer to read data into.
 * @param	size			Amount of data to read.
 * @param	timeout			Timeout after which to return.
 * @return	Amount of data read.
 */
int serial_read(int fd, char* data, size_t size, double timeout)
{
	int read_size = 0;
	int read_result = 0;
	fd_set read_fds;
	int selected_fd;
	struct timeval timeout_struct;

	/* Initialize the input set */
	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	/* Initialize the timeout structure */
	timeout_struct.tv_sec = rint(timeout);
	timeout_struct.tv_usec = rint((timeout - round(timeout)) * pow(10, 6));

	// Loop while not all data has been retrieved.
	while (read_size < size)
	{
		/* Do the select */
		selected_fd = select(fd + 1, &read_fds,  NULL, NULL, &timeout_struct);

		/* See if there was an error */
		if (selected_fd < 0) {
			fprintf(stderr, "Failed to select a file descriptor: %s.\n", strerror(errno));
			break;
		}
		else if (selected_fd == 0) {
			fprintf(stderr, "A timeout occurred.\n");
			break;
		}
		else
		{
			read_result = read(fd, &(data[read_size]), size - read_size);
			if (read_result < 0)
			{
				fprintf(stderr, "Failed to read data: %s.\n", strerror(errno));
				break;
			}
			read_size += read_result;
		}
	}

	size_t i;
	printf("Reading: ");
	for (i = 0; i < read_size; i++) {
		printf("0x%02x ", (unsigned char) data[i]);
	}
	printf("\n");

	return read_size;
}

/**
 * @brief	Set the RTS pin status.
 * @param	fd				File descriptor for the serial port.
 * @param	enabled			TRUE = enabled, FALSE = disabled.
 * @return	None.
 */
void serial_set_rts(int fd, bool enabled)
{
	int status;

	// Get the current pin status.
	ioctl(fd, TIOCMGET, &status);

	// Configure the pin status.
	if (!enabled)
	{
		status |= TIOCM_RTS;
	}
	else
	{
		status &= ~TIOCM_RTS;
	}

	// Set the pin status.
	ioctl(fd, TIOCMSET, &status);
}

/**
 * @brief	Set the DTR pin status.
 * @param	fd				File descriptor for the serial port.
 * @param	enabled			TRUE = enabled, FALSE = disabled.
 * @return	None.
 */
void serial_set_dtr(int fd, bool enabled)
{
	int status;

	// Get the current pin status.
	ioctl(fd, TIOCMGET, &status);

	// Configure the pin status.
	if (!enabled)
	{
		status |= TIOCM_DTR;
	}
	else
	{
		status &= ~TIOCM_DTR;
	}

	// Set the pin status.
	ioctl(fd, TIOCMSET, &status);
}

/**
 * @brief	Get the CTS pin status.
 * @param	fd File descriptor for the serial port.
 * @return	CTS pin status.
 */
bool serial_get_cts(int fd)
{
	int status;

	// Get the current pin status.
	ioctl(fd, TIOCMGET, &status);

	printf("%d\n", status);

	return (status & TIOCM_CTS);
}

/**
 * @brief	Get the DCD pin status.
 * @param	fd				File descriptor for the serial port.
 * @return	DCD pin status.
 */
bool serial_get_dcd(int fd)
{
	int status;

	// Get the current pin status.
	ioctl(fd, TIOCMGET, &status);

	return (status & TIOCM_CAR);
}

/**
 * @brief	Get the DSR pin status.
 * @param	fd File descriptor for the serial port.
 * @return	DSR pin status.
 */
bool serial_get_dsr(int fd)
{
	int status;

	// Get the current pin status.
	ioctl(fd, TIOCMGET, &status);

	printf("%d\n", status);

	return (status & TIOCM_DSR);
}

/**
 * @brief	Change the baudrate of the serial port.
 * @param	fd File descriptor for the serial port.
 * @param	baudrate The desired baudrate.
 * @return	None.
 */
void serial_change_baudrate(int fd, serial_baudrate baudrate)
{
	struct termios options;
	speed_t baud;

	// Get the current options.
	tcgetattr(fd, &options);

	// Set the baud rate.
	baud = _serial_get_baudrate(baudrate);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);

	// Write the options to the port.
	tcsetattr(fd, TCSANOW, &options);
}

/**
 * @brief	Calculate the serial baud rate constants.
 * @param	baudrate Enum with baud rate.
 * @return	Baud rate constants..
 */
static speed_t _serial_get_baudrate(serial_baudrate baudrate)
{
	speed_t baudrate_constant;

	switch(baudrate)
	{
	case baudrate_0:
		baudrate_constant = B0;
		break;
	case baudrate_50:
		baudrate_constant = B50;
		break;
	case baudrate_75:
		baudrate_constant = B75;
		break;
	case baudrate_110:
		baudrate_constant = B110;
		break;
	case baudrate_134:
		baudrate_constant = B134;
		break;
	case baudrate_150:
		baudrate_constant = B150;
		break;
	case baudrate_200:
		baudrate_constant = B200;
		break;
	case baudrate_300:
		baudrate_constant = B300;
		break;
	case baudrate_600:
		baudrate_constant = B600;
		break;
	case baudrate_1200:
		baudrate_constant = B1200;
		break;
	case baudrate_1800:
		baudrate_constant = B1800;
		break;
	case baudrate_2400:
		baudrate_constant = B2400;
		break;
	case baudrate_4800:
		baudrate_constant = B4800;
		break;
	case baudrate_9600:
		baudrate_constant = B9600;
		break;
	case baudrate_19200:
		baudrate_constant = B19200;
		break;
	case baudrate_38400:
		baudrate_constant = B38400;
		break;
	case baudrate_57600:
		baudrate_constant = B57600;
		break;
	case baudrate_115200:
		baudrate_constant = B115200;
		break;
	}

	return baudrate_constant;
}

/**
 * @}
 */
