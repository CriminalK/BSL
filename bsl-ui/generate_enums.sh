#/bin/bash

glib-mkenums --fhead "#ifndef __BSL_TYPE_BUILTINS_H__\n#define __BSL_TYPE_BUILTINS_H__\n\n#include <gtk/gtk.h>\n" \
             --fprod "\n// Enumerations from \"@filename@\".\n" \
             --vhead "#define BSL_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\nGType @enum_name@_get_type(void);\n" \
             --ftail "\n#endif /* __BSL_TYPE_BUILTINS_H__ */" \
             bsl-controls.h \
             > bsl-enum-types.h

glib-mkenums --fhead "#include <gtk/gtk.h>\n#include \"bsl-enum-types.h\"\n" \
             --fprod "\n// Enumerations from \"@filename@\".\n#include \"@filename@\"\n\n" \
             --vhead "GType @enum_name@_get_type(void)\n{\n\tstatic GType etype = 0;\n\tif (etype == 0) {\n\t\tstatic const G@Type@Value values[] = {" \
             --vprod "\t\t\t{ @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\"}," \
             --vtail "\t\t\t{ 0, NULL, NULL }\n\t\t};\n\t\tetype = g_@type@_register_static(\"@EnumName@\", values);\n\t}\n\treturn etype;\n}\n" \
             bsl-controls.h \
             > bsl-enum-types.c
