/* unistd.h - Windows stub for MSVC (no-op on other platforms) */
#ifdef _MSC_VER
    #ifndef _unistd_h_
    #define _unistd_h_
    #include <io.h>
    #include <direct.h>
    #include <process.h>
    #endif
#else
    /* On Unix/macOS, include the real system unistd.h */
    #include_next <unistd.h>
#endif
