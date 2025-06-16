#define Z_STRING_IMPLEMENTATION
#define Z_STOPWATCH_IMPLEMENTATION
#define Z_THREADING_IMPLEMENTATION

#include "./zenlib_core.h"

ZEN_CoreContext __zencore_context__ = (ZEN_CoreContext) {
    .renderer_context.render_object_count = 0,
    .renderer_context.render_object_capacity = 0,
    .renderer_context.render_object_last_count = 0,
    .window_count = 0
};