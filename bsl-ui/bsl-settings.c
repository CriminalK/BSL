/*
 * bsl-settings.c
 *
 *  Created on: 1 mrt. 2015
 *      Author: enjschreuder
 */

#include <gtk/gtk.h>
#include "bsl-settings.h"

typedef struct _BslSettingsPrivate {
	GtkWidget *	bsl_version_status_label;
	GtkWidget *	chip_id_status_label;
	GtkWidget *	info_frame;
	GtkWidget *	configuration_frame;
	GtkWidget *	connect_toggle_button;
} BslSettingsPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(BslSettings, bsl_settings, GTK_TYPE_BIN)

static void bsl_settings_connect_toggle_button_cb(GtkToggleButton *togglebutton, gpointer user_data);

static void bsl_settings_class_init(BslSettingsClass * class)
{
	class->connect_signal = g_signal_new(	"connect"						/* signal_name,*/,
											G_TYPE_FROM_CLASS(class)		/* itype */,
											G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS /* signal_flags */,
											0								/* class_offset */,
											NULL							/* accumulator */,
											NULL							/* accu_data */,
											g_cclosure_marshal_VOID__VOID	/* c_marshaller */,
											G_TYPE_NONE						/* return_type */,
											0								/* n_params */);

	class->disconnect_signal = g_signal_new(	"disconnect"					/* signal_name,*/,
												G_TYPE_FROM_CLASS(class)		/* itype */,
												G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS /* signal_flags */,
												0								/* class_offset */,
												NULL							/* accumulator */,
												NULL							/* accu_data */,
												g_cclosure_marshal_VOID__VOID	/* c_marshaller */,
												G_TYPE_NONE						/* return_type */,
												0								/* n_params */);
}

static void bsl_settings_init(BslSettings * bsl_settings)
{
	BslSettingsPrivate * priv;
	priv = bsl_settings_get_instance_private(bsl_settings);

	GtkWidget * vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	priv->configuration_frame = gtk_frame_new("BSL Configuration");

	GtkWidget * configuration_grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(configuration_grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(configuration_grid), 5);

	priv->connect_toggle_button = gtk_toggle_button_new_with_label("Connect");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->connect_toggle_button), FALSE);

	GtkWidget * file_button = gtk_file_chooser_button_new("Select current firmware file", GTK_FILE_CHOOSER_ACTION_OPEN);

	GtkWidget * serial_combobox = gtk_combo_box_new();

	GtkWidget * serial_port_label = gtk_label_new("Serial port:");
	GtkWidget * password_file_label = gtk_label_new("Password file:");

	gtk_grid_attach(GTK_GRID(configuration_grid), serial_port_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(configuration_grid), serial_combobox, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(configuration_grid), priv->connect_toggle_button, 2, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(configuration_grid), password_file_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(configuration_grid), file_button, 1, 1, 2, 1);

	gtk_container_add(GTK_CONTAINER(priv->configuration_frame), configuration_grid);

	priv->info_frame = gtk_frame_new("BSL Information");

	GtkWidget * info_grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(info_grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(info_grid), 5);

	GtkWidget * bsl_version_label = gtk_label_new("BSL Version:");
	priv->bsl_version_status_label = gtk_label_new("");
	GtkWidget * chip_id_label = gtk_label_new("Chip ID:");
	priv->chip_id_status_label = gtk_label_new("");

	gtk_grid_attach(GTK_GRID(info_grid), bsl_version_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(info_grid), priv->bsl_version_status_label, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(info_grid), chip_id_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(info_grid), priv->chip_id_status_label, 1, 1, 1, 1);

	gtk_container_add(GTK_CONTAINER(priv->info_frame), info_grid);

	gtk_box_pack_start(GTK_BOX(vertical_box), priv->configuration_frame, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vertical_box), priv->info_frame, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(bsl_settings), vertical_box);

	gtk_widget_set_sensitive(priv->info_frame, FALSE);

	g_signal_connect(priv->connect_toggle_button, "clicked", G_CALLBACK(bsl_settings_connect_toggle_button_cb), bsl_settings);
}

GtkWidget * bsl_settings_new(void)
{
	return GTK_WIDGET(g_object_new(BSL_TYPE_SETTINGS, NULL));
}

void bsl_settings_set_information(BslSettings * bsl_settings, BslSettingsInformation * bsl_settings_information)
{
	BslSettingsPrivate * priv;
	priv = bsl_settings_get_instance_private(bsl_settings);

	gtk_label_set_text(GTK_LABEL(priv->bsl_version_status_label), bsl_settings_information->bsl_version);
	gtk_label_set_text(GTK_LABEL(priv->chip_id_status_label), bsl_settings_information->chip_id);
}

void bsl_settings_set_connected(BslSettings * bsl_settings, gboolean connected)
{
	BslSettingsPrivate * priv;
	priv = bsl_settings_get_instance_private(bsl_settings);

	/* If the device is connected, lock enable the information frame,
	 * otherwise, empty the information fields and disable it. */
	if (connected) {
		gtk_widget_set_sensitive(priv->info_frame, TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->connect_toggle_button), TRUE);
	}
	else {
		gtk_label_set_text(GTK_LABEL(priv->bsl_version_status_label), "");
		gtk_label_set_text(GTK_LABEL(priv->chip_id_status_label), "");
		gtk_widget_set_sensitive(priv->info_frame, FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->connect_toggle_button), FALSE);
	}
}

static void bsl_settings_connect_toggle_button_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
	BslSettings * bsl_settings = (BslSettings *) user_data;
	BslSettingsPrivate * priv;
	priv = bsl_settings_get_instance_private(bsl_settings);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->connect_toggle_button))) {
		g_signal_emit(bsl_settings, BSL_SETTINGS_GET_CLASS(bsl_settings)->connect_signal, 0);
	}
	else {
		g_signal_emit(bsl_settings, BSL_SETTINGS_GET_CLASS(bsl_settings)->disconnect_signal, 0);
	}
}
