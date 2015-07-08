


#include <gtk/gtk.h>
#include "bsl-enum-types.h"

// Enumerations from "bsl-controls.h".
#include "bsl-controls.h"

GType bsl_controls_command_get_type(void)
{
	static GType etype = 0;
	if (etype == 0) {
		static const GEnumValue values[] = {
			{ BSL_CONTROLS_COMMAND_READ, "BSL_CONTROLS_COMMAND_READ", "read"},
			{ BSL_CONTROLS_COMMAND_WRITE, "BSL_CONTROLS_COMMAND_WRITE", "write"},
			{ BSL_CONTROLS_COMMAND_VALIDATE, "BSL_CONTROLS_COMMAND_VALIDATE", "validate"},
			{ BSL_CONTROLS_COMMAND_ERASE, "BSL_CONTROLS_COMMAND_ERASE", "erase"},
			{ 0, NULL, NULL }
		};
		etype = g_enum_register_static("BslControlsCommand", values);
	}
	return etype;
}



