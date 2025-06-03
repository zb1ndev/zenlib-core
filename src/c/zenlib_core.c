#define Z_STRING_IMPLEMENTATION

#include "../include/zstring.h"
#include "../include/zenlib_core.h"

ZEN_CoreContext __zencore_context__ = (ZEN_CoreContext) {
    .render_object_count = 0,
    .render_object_capacity = 0,
    .render_object_last_count = 0,
    .window_count = 0
};