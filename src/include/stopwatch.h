#if !defined(STOPWATCH_H)
#define STOPWATCH_H

    #include <time.h>

    typedef struct ZEN_Stopwatch {
        clock_t start_time;
        clock_t end_time;
        int running;
    } ZEN_Stopwatch;

    void zen_stopwatch_start(ZEN_Stopwatch* sw);
    void zen_stopwatch_stop(ZEN_Stopwatch* sw);
    double zen_stopwatch_elapsed_seconds(ZEN_Stopwatch* sw);
    double zen_stopwatch_elapsed_milliseconds(ZEN_Stopwatch* sw);

#endif // STOPWATCH_H