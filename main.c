/*
 * main.c
 *
 *  Created on: 11 feb. 2015
 *      Author: enjschreuder
 */

#include "serial.h"
#include "bsl.h"
#include "device.h"
#include "ihex.h"
#include <stdio.h>

#include <gtk/gtk.h>
#include "bsl-ui/bsl-window.h"

int main(int argc, char *argv[])
{
	ihex_t * ihex = ihex_create();

	ihex_read_file(ihex, "hex.hex");

	ihex_print(ihex);

	return 0;
}


//int main(int argc, char *argv[])
//{
//	gtk_init (&argc, &argv);
//
//	BslWindow * window = bsl_window_new();
//
//	g_signal_connect(GTK_WIDGET(window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
//
//	gtk_widget_show_all(GTK_WIDGET(window));
//
//	gtk_main();
//
//	return 0;
//}

//
//int main(void)
//{
//	int error = 0;
//	int fd;
//	bsl_object_t * bsl_object_p;
//	device_object_t * device_object_p;
//	size_t i;
//	const unsigned char password[] =
//	{0x88, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x8C, 0xC1, 0xBE, 0xC1,
//	0x88, 0xC1, 0x88, 0xC1, 0xF2, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x88, 0xC1, 0x00, 0xC0};
//
//	// Setup the serial settings.
//	serial_settings_t serial_settings;
//	serial_settings.baudrate = baudrate_9600;
//	serial_settings.databits = databits_8;
//	serial_settings.parity = even;
//	serial_settings.stopbits = stopbits_1;
//	serial_settings.flow_control = false;
//
//	// Open the serial port.
//	fd = serial_open("/dev/ttyUSB0", serial_settings);
//
//	if (fd == -1)
//	{
//		error = 1;
//	}
//
//	if (!error)
//	{
//		// Initialize the BSL.
//		bsl_object_p = bsl_initialize(fd);
//		if (bsl_object_p == NULL)
//		{
//			error = 1;
//		}
//	}
//
//	if (!error)
//	{
//		// Initialize device.
//		device_object_p = device_initialize(bsl_object_p, password);
//		if (device_object_p == NULL)
//		{
//			error = 1;
//		}
//	}
//
//	if (!error)
//	{
//		// Receive the information memory.
//		unsigned char information_memory[256];
//
//		error = device_read_memory(device_object_p, 0x1000, information_memory, 256);
//		if (!error)
//		{
//			printf("Information memory: ");
//			for (i = 0; i < sizeof(information_memory); i++)
//			{
//				printf("%02x ", information_memory[i]);
//			}
//			printf("\n");
//		}
//	}
//
//	if (device_object_p != NULL)
//	{
//		device_terminate(device_object_p);
//	}
//
//	if (bsl_object_p != NULL) {
//		bsl_terminate(bsl_object_p);
//	}
//
//	if (fd != -1) {
//		serial_close(fd);
//	}
//
//	return 0;
//}
