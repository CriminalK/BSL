/*
 * bsl_window.c
 *
 *  Created on: 1 mrt. 2015
 *      Author: enjschreuder
 */

#include <gtk/gtk.h>

#include "bsl-window.h"
#include "bsl-settings.h"
#include "bsl-controls.h"

// Box spacing.
#define BSL_WINDOW_BOX_SPACING		(0)
#define BSL_WINDOW_BOX_PADDING		(0)
#define BSL_WINDOW_BORDER_WIDTH		(10)
#define BSL_WINDOW_DEFAULT_WIDTH	(400)
#define BSL_WINDOW_DEFAULT_HEIGHT	(500)

typedef struct _BslWindowPrivate
{
	GtkWidget *	bsl_settings;
	GtkWidget * bsl_controls;
} BslWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(BslWindow, bsl_window, GTK_TYPE_WINDOW);

static void bsl_window_connect_cb(GtkWidget * bsl_settings, gpointer user_data);

BslWindow * bsl_window_new()
{
	return g_object_new(BSL_WINDOW_TYPE, "type", GTK_WINDOW_TOPLEVEL, NULL);
}

static void bsl_window_class_init(BslWindowClass *class)
{
}

static void bsl_window_init(BslWindow *window)
{
	BslWindowPrivate * priv;
	priv = bsl_window_get_instance_private(window);

	// Apply settings for the window.
	gtk_window_set_title(GTK_WINDOW(window), "BSL Application");
	gtk_container_set_border_width(GTK_CONTAINER(window), BSL_WINDOW_BORDER_WIDTH);
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	gtk_window_set_default_size (GTK_WINDOW(window), BSL_WINDOW_DEFAULT_WIDTH, BSL_WINDOW_DEFAULT_HEIGHT);

	// Add a box to the window.
	priv->bsl_settings = bsl_settings_new();
	priv->bsl_controls = bsl_controls_new();

	// Add a new vertical box containing all widgets.
	GtkWidget * vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_pack_start(GTK_BOX(vertical_box), priv->bsl_settings, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vertical_box), priv->bsl_controls, FALSE, FALSE, 0);

	// Set all widgets except for the bsl_settings to not sensitive.
	gtk_widget_set_sensitive(priv->bsl_controls, FALSE);

	// Add the vertical box to the window.
	gtk_container_add(GTK_CONTAINER(window), vertical_box);

	g_signal_connect(priv->bsl_settings, "connect", G_CALLBACK(bsl_window_connect_cb), window);
}

static void bsl_window_connect_cb(GtkWidget * bsl_settings, gpointer user_data)
{
	BslWindow * window = (BslWindow *) user_data;
	BslWindowPrivate * priv;
	priv = bsl_window_get_instance_private(window);

	bsl_settings_set_connected(BSL_SETTINGS(bsl_settings), TRUE);
	BslSettingsInformation bsl_settings_information;
	bsl_settings_information.bsl_version = "2.2";
	bsl_settings_information.chip_id = "1000";
	bsl_settings_set_information(BSL_SETTINGS(bsl_settings), &bsl_settings_information);

	gtk_widget_set_sensitive(priv->bsl_controls, TRUE);
}
