#include "../include/stopwatch.h"

void zen_stopwatch_start(ZEN_Stopwatch* sw) {
    sw->start_time = clock();
    sw->running = 1;
}

void zen_stopwatch_stop(ZEN_Stopwatch* sw) {
    sw->end_time = clock();
    sw->running = 0;
}

double zen_stopwatch_elapsed_seconds(ZEN_Stopwatch* sw) {
    clock_t end = sw->running ? clock() : sw->end_time;
    return (double)(end - sw->start_time) / CLOCKS_PER_SEC;
}

double zen_stopwatch_elapsed_milliseconds(ZEN_Stopwatch* sw) {
    return zen_stopwatch_elapsed_seconds(sw) * 1000.0;
}