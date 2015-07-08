/*
 * bsl-controls.h
 *
 *  Created on: 1 mrt. 2015
 *      Author: enjschreuder
 */

#ifndef __BSL_CONTROLS_H__
#define __BSL_CONTROLS_H__

#include <gtk/gtk.h>

#define BSL_TYPE_CONTROLS			(bsl_controls_get_type ())
#define BSL_CONTROLS(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), BSL_TYPE_CONTROLS, BslControls))
#define BSL_CONTROLS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), BSL_TYPE_CONTROLS, BslControlsClass))

typedef struct _BslControls			BslControls;
typedef struct _BslControlsClass	BslControlsClass;

struct _BslControls
{
	GtkBin	parent_instance;
};

struct _BslControlsClass
{
	GtkBinClass	parent_class;

	guint		command_signal;
};

GType bsl_controls_get_type(void);
GtkWidget * bsl_controls_new(void);

typedef enum
{
	BSL_CONTROLS_COMMAND_READ,
	BSL_CONTROLS_COMMAND_WRITE,
	BSL_CONTROLS_COMMAND_VALIDATE,
	BSL_CONTROLS_COMMAND_ERASE
} BslControlsCommand;

void bsl_controls_set_progress(BslControls * bsl_controls, gdouble fraction, BslControlsCommand command, gboolean complete);

#endif /* __BSL_COTNROLS_H__ */

