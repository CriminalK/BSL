/*
 * bsl_window.h
 *
 *  Created on: 1 mrt. 2015
 *      Author: enjschreuder
 */

#ifndef BSL_WINDOW_H_
#define BSL_WINDOW_H_

#include <gtk/gtk.h>

#define BSL_WINDOW_TYPE (bsl_window_get_type ())
#define BSL_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BSL_WINDOW_TYPE, BslWindow))

typedef struct _BslWindow		BslWindow;
typedef struct _BslWindowClass	BslWindowClass;

struct _BslWindow
{
  GtkWindow parent;
};

struct _BslWindowClass
{
  GtkWindowClass parent_class;
};

GType bsl_window_get_type(void);
BslWindow * bsl_window_new(void);

#endif /* BSL_WINDOW_H_ */
