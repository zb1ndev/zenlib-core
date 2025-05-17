#if !defined(PLATFORM_H)
#define PLATFORM_H

    // Windows
    #if defined(_WIN32) || defined(_WIN64)
        #define ZEN_OS_WINDOWS
    #endif

    // macOS
    #if defined(__APPLE__) && defined(__MACH__)
        #define ZEN_OS_MAC
    #endif

    // Linux
    #if defined(__linux__)
        #define ZEN_OS_LINUX
    #endif

    // Unix (includes macOS and Linux)
    #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
        #define ZEN_OS_UNIX
    #endif

#endif // PLATFORM_H