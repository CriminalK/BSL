/*
 * bsl-controls.c
 *
 *  Created on: 1 mrt. 2015
 *      Author: enjschreuder
 */

#include <gtk/gtk.h>
#include "bsl-controls.h"
#include "bsl-enum-types.h"

typedef struct _BslControlsPrivate {
	GtkWidget *	bsl_progress_bar;
	GtkWidget * control_button_box;
	GtkWidget * read_button;
	GtkWidget * write_button;
	GtkWidget * validate_button;
	GtkWidget * erase_button;
} BslControlsPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(BslControls, bsl_controls, GTK_TYPE_BIN)

static void bsl_controls_read_button_cb(GtkButton *button, gpointer user_data);
static void bsl_controls_write_button_cb(GtkButton *button, gpointer user_data);
static void bsl_controls_validate_toggle_button_cb(GtkButton *button, gpointer user_data);
static void bsl_controls_erase_toggle_button_cb(GtkButton *button, gpointer user_data);
static void bsl_controls_button_cb(GtkButton *button, BslControlsCommand command, gpointer user_data);

static void bsl_controls_class_init(BslControlsClass * class)
{
	class->command_signal = g_signal_new(	"command"						/* signal_name,*/,
											G_TYPE_FROM_CLASS(class)		/* itype */,
											G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS /* signal_flags */,
											0								/* class_offset */,
											NULL							/* accumulator */,
											NULL							/* accu_data */,
											g_cclosure_marshal_VOID__ENUM	/* c_marshaller */,
											G_TYPE_NONE						/* return_type */,
											1								/* n_params */,
											BSL_TYPE_CONTROLS_COMMAND);
}

static void bsl_controls_init(BslControls * bsl_controls)
{
	BslControlsPrivate * priv;
	priv = bsl_controls_get_instance_private(bsl_controls);

	GtkWidget * operation_frame = gtk_frame_new("Operation");

	priv->bsl_progress_bar = gtk_progress_bar_new();

	priv->control_button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);

	priv->read_button = gtk_button_new_with_label("Read");
	priv->write_button = gtk_button_new_with_label("Write");
	priv->validate_button = gtk_button_new_with_label("Validate");
	priv->erase_button = gtk_button_new_with_label("Erase");

	gtk_box_pack_start(GTK_BOX(priv->control_button_box), priv->read_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(priv->control_button_box), priv->write_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(priv->control_button_box), priv->validate_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(priv->control_button_box), priv->erase_button, FALSE, FALSE, 0);

	g_signal_connect(priv->read_button, "clicked", G_CALLBACK(bsl_controls_read_button_cb), bsl_controls);
	g_signal_connect(priv->write_button, "clicked", G_CALLBACK(bsl_controls_write_button_cb), bsl_controls);
	g_signal_connect(priv->validate_button, "clicked", G_CALLBACK(bsl_controls_validate_toggle_button_cb), bsl_controls);
	g_signal_connect(priv->erase_button, "clicked", G_CALLBACK(bsl_controls_erase_toggle_button_cb), bsl_controls);

	GtkWidget * vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_pack_start(GTK_BOX(vertical_box), priv->bsl_progress_bar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vertical_box), priv->control_button_box, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(operation_frame), vertical_box);
	gtk_container_add(GTK_CONTAINER(bsl_controls), operation_frame);
}

GtkWidget * bsl_controls_new(void)
{
	return GTK_WIDGET(g_object_new(BSL_TYPE_CONTROLS, NULL));
}

void bsl_controls_set_progress(BslControls * bsl_controls, gdouble fraction, BslControlsCommand command, gboolean complete)
{
	BslControlsPrivate * priv;
	priv = bsl_controls_get_instance_private(bsl_controls);

	if (!complete) {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->bsl_progress_bar), fraction);

		gchar * text;
		switch (command)
		{
		case BSL_CONTROLS_COMMAND_READ:
			text = "Reading...";
			break;
		case BSL_CONTROLS_COMMAND_WRITE:
			text = "Writing...";
			break;
		case BSL_CONTROLS_COMMAND_VALIDATE:
			text = "Validating...";
			break;
		case BSL_CONTROLS_COMMAND_ERASE:
			text = "Erasing...";
			break;
		}
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(priv->bsl_progress_bar), text);
	}
	else {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->bsl_progress_bar), 0.0);
		gtk_widget_set_sensitive(priv->control_button_box, TRUE);
	}
}

static void bsl_controls_read_button_cb(GtkButton *button, gpointer user_data)
{
	bsl_controls_button_cb(button, BSL_CONTROLS_COMMAND_READ, user_data);
}

static void bsl_controls_write_button_cb(GtkButton *button, gpointer user_data)
{
	bsl_controls_button_cb(button, BSL_CONTROLS_COMMAND_WRITE, user_data);
}

static void bsl_controls_validate_toggle_button_cb(GtkButton *button, gpointer user_data)
{
	bsl_controls_button_cb(button, BSL_CONTROLS_COMMAND_VALIDATE, user_data);
}

static void bsl_controls_erase_toggle_button_cb(GtkButton *button, gpointer user_data)
{
	bsl_controls_button_cb(button, BSL_CONTROLS_COMMAND_ERASE, user_data);
}

static void bsl_controls_button_cb(GtkButton *button, BslControlsCommand command, gpointer user_data)
{
	BslControls * bsl_controls = (BslControls *) user_data;
	BslControlsPrivate * priv;
	priv = bsl_controls_get_instance_private(bsl_controls);

	// Disable the button box.
	gtk_widget_set_sensitive(priv->control_button_box, FALSE);

	// Emit a signal that a command is issued.
	g_signal_emit(user_data, BSL_CONTROLS_GET_CLASS(user_data)->command_signal, 0, command);
}
