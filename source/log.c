#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#if defined(__linux__)
    #define __GNU_SOURCE
    #include <sys/syscall.h>
#elif defined(__MACH__) || defined(__APPLE__)
    #include <pthread.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"

static void get_tid(int *tid) {
    #if defined(__linux__)
        *tid = syscall(SYS_gettid);
    #elif defined(_WIN32) || defined(_WIN64)
        *tid = GetCurrentThreadId();
    #elif defined(__MACH__) || defined(__APPLE__)
        *tid = (int)pthread_mach_thread_np(pthread_self());
    #else
        *tid = -1;
    #endif
    return;
}

static void get_pid(int *pid) {
    #if defined(_WIN32) || defined(_WIN64)
        *pid = GetCurrentProcessId();
    #else
        *pid = getpid();
    #endif
    return;
}

static const char *level_to_str(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_NONE: return "";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_CRITICAL: return "CRITICAL";
    }
    return "";
}

static void gen_filename(LogFileHandler *handler, uint8_t idx) {
    char suffix[LOG_ROLL_OVER_SUFFIX_LEN]; // "_255'0'"
    char *suffix_start;
    snprintf(suffix, sizeof(suffix), "_%u", idx);

    suffix_start = strrchr(handler->filepath, '_');
    if (suffix_start) {
        strncpy(suffix_start, suffix, sizeof(suffix) - 1);
        suffix_start[sizeof(suffix) - 1] = '\0';
    }
    else {
        strncat(handler->filepath, suffix, sizeof(handler->filepath) - strlen(handler->filepath) - 1);
    }
}

void log_kill_file_handler(LogFileHandler *handler) {
    if (handler->stream) {
        fclose(handler->stream);
    }
    return;
}

int log_init_file_handler(const char *filepath, long maxsize, LogFileHandler *handler) {
    if (handler->stream) {
        log_kill_file_handler(handler);
    }

    handler->stream = fopen(filepath, "a");
    if (!handler->stream) {
        return -1;
    }

    snprintf(handler->filepath, sizeof(handler->filepath), "%s", filepath);

    if (maxsize <= 0) {
        handler->maxsize = LOG_DEFAULT_MAX_FILESIZE;
    }
    else {
        handler->maxsize = maxsize;
    }
    
    return 0;
}

void log_to_file(const char *file, const char *func, int line, LogFileHandler *handler, LogLevel level, uint8_t flags, const char *fmt, ...) {
    if (!handler->stream) return;

    char out[LOG_BUFSIZE];
    const char *level_str;
    int out_idx;
    time_t t;
    struct tm *tm;

    va_list args;
    va_start(args, fmt);

    t = time(NULL);
    tm = localtime(&t);

    out_idx = 0;
    level_str = level_to_str(level);

    if (flags & LOG_DATE) {
        out_idx += strftime(out + out_idx, sizeof(out) - out_idx, "%Y-%m-%d", tm);
    } 
    if (flags & LOG_TIME) {
        out_idx += strftime(out + out_idx, sizeof(out) - out_idx, " %H:%M:%S", tm);
    }
    if (level_str) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " %s", level_str);
    }
    if (flags & LOG_FILE) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " [%s]", file);
    }
    if (flags & LOG_FUNC) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, ":%s", func);
    }
    if (flags & LOG_LINE) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, ":%d", line);
    }
    if (flags & LOG_TID) {
        int tid;
        get_tid(&tid);
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " tid:%d", tid);
    }
    if (flags & LOG_PID) {
        int pid;
        get_pid(&pid);
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " pid:%d", pid);
    }
    if (flags & LOG_ROLL_OVER) {
        long filesize;
        uint8_t i;

        filesize = ftell(handler->stream);
        if (filesize > handler->maxsize && strlen(handler->filepath) + LOG_ROLL_OVER_SUFFIX_LEN + 1 < sizeof(handler->filepath)) {
            for (i = 0; i < LOG_ROLL_OVER_MAX; i++) {
                gen_filename(handler, i);
                log_init_file_handler(handler->filepath, handler->maxsize, handler);
                filesize = ftell(handler->stream);
                if (filesize < handler->maxsize) {
                    break;
                }
            }
            if (!handler->stream) {
                goto end; // sorry no log today
            }
        }
    }
    fprintf(handler->stream, "%s\n", out);
    vfprintf(handler->stream, fmt, args);
    if (level == LOG_LEVEL_WARNING || level == LOG_LEVEL_CRITICAL) {
        fprintf(handler->stream, "\nERRNO: %s\n", strerror(errno));
    }
    fprintf(handler->stream, "\n");
    fflush(handler->stream);
end:
    va_end(args);
}

void log_to_stream(const char *file, const char *func, int line, FILE *stream, LogLevel level, uint8_t flags, const char *fmt, ...) {
    if (!stream) return;

    char out[LOG_BUFSIZE];
    const char *level_str;
    int out_idx;
    time_t t;
    struct tm *tm;

    va_list args;
    va_start(args, fmt);

    t = time(NULL);
    tm = localtime(&t);

    out_idx = 0;
    level_str = level_to_str(level);

    if (flags & LOG_DATE) {
        out_idx += strftime(out + out_idx, sizeof(out) - out_idx, "%Y-%m-%d", tm);
    } 
    if (flags & LOG_TIME) {
        out_idx += strftime(out + out_idx, sizeof(out) - out_idx, " %H:%M:%S", tm);
    }
    if (level_str) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " %s", level_str);
    }
    if (flags & LOG_FILE) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " [%s]", file);
    }
    if (flags & LOG_FUNC) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, ":%s", func);
    }
    if (flags & LOG_LINE) {
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, ":%d", line);
    }
    if (flags & LOG_TID) {
        int tid;
        get_tid(&tid);
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " tid:%d", tid);
    }
    if (flags & LOG_PID) {
        int pid;
        get_pid(&pid);
        out_idx += snprintf(out + out_idx, sizeof(out) - out_idx, " pid:%d", pid);
    }
   
    fprintf(stream, "%s\n", out);
    vfprintf(stream, fmt, args);
    if (level == LOG_LEVEL_WARNING || level == LOG_LEVEL_CRITICAL) {
        fprintf(stream, "\nERRNO: %s\n", strerror(errno));
    }
    fprintf(stream, "\n");
    fflush(stream);
    va_end(args);
}