#if !defined(Z_STOPWATCH_H)
#define Z_STOPWATCH_H

    #include <time.h>

    typedef struct ZEN_Stopwatch {
        clock_t start_time;
        clock_t end_time;
        int running;
    } ZEN_Stopwatch;

    /** A function that starts the stopwatch timer.
     * @param sw The stopwatch instance you want to start.
     */
    void zen_stopwatch_start(ZEN_Stopwatch* sw);

    /** A function that stops the stopwatch timer.
     * @param sw The stopwatch instance you want to stop.
     */
    void zen_stopwatch_stop(ZEN_Stopwatch* sw);

    /** A function that returns the elapsed time in seconds.
     * @param sw The stopwatch instance you want to query.
     * @returns The elapsed time in seconds as a double.
     */
    double zen_stopwatch_elapsed_seconds(ZEN_Stopwatch* sw);

    /** A function that returns the elapsed time in milliseconds.
     * @param sw The stopwatch instance you want to query.
     * @returns The elapsed time in milliseconds as a double.
     */
    double zen_stopwatch_elapsed_milliseconds(ZEN_Stopwatch* sw);

#if defined(Z_STOPWATCH_IMPLEMENTATION)

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

#endif // Z_STOPWATCH_IMPLEMENTATION
#endif // Z_STOPWATCH_H