#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/* HELPER MACROS AROUND LIBRARY METHODS */
#define LOG_TO_FILE(handler, level, flags, fmt, ...) \
  log_to_file(__FILE__, __func__, __LINE__, (handler), (level), (flags), (fmt), ##__VA_ARGS__)

#define LOG_TO_STREAM(stream, level, flags, fmt, ...) \
  log_to_stream(__FILE__, __func__, __LINE__, (stream), (level), (flags), (fmt), ##__VA_ARGS__)

/* BITFIELD FOR FLAGS */
#define LOG_DATE (1 << 0)      // 0b00000001
#define LOG_TIME (1 << 1)      // 0b00000010
#define LOG_FILE (1 << 2)      // 0b00000100
#define LOG_FUNC (1 << 3)      // 0b00001000
#define LOG_LINE (1 << 4)      // 0b00010000
#define LOG_TID (1 << 5)       // 0b00100000
#define LOG_PID (1 << 6)       // 0b01000000
#define LOG_ROLL_OVER (1 << 7) // 0b10000000 (only for logging to file)

#define LOG_ALL_FILE (LOG_TIME | LOG_DATE | LOG_FILE | LOG_FUNC | LOG_LINE | LOG_TID | LOG_PID | LOG_ROLL_OVER)
#define LOG_ALL (LOG_TIME | LOG_DATE | LOG_FILE | LOG_FUNC | LOG_LINE | LOG_TID | LOG_PID)

/* ROLL_OVER CONFIG */
#define LOG_ROLL_OVER_MAX ((uint8_t)(10))
#define LOG_ROLL_OVER_SUFFIX_LEN 5

/* MAX LOG BUFFER */
#define LOG_BUFSIZE 1024

/* MAX FILESIZE */
#define LOG_DEFAULT_MAX_FILESIZE ((long)(5 * 1024 * 1024)) // 5 MB

/* MAX LENGTH FOR FILENAME */
#define LOG_MAX_FILENAME_LEN 255

/* LEVELS */
typedef enum {
  LOG_LEVEL_NONE,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_CRITICAL // CHECKS ERRNO
} LogLevel;

typedef struct {
  FILE *stream;
  char filepath[LOG_MAX_FILENAME_LEN];
  long maxsize;
} LogFileHandler;

void log_kill_file_handler(LogFileHandler *handler);
int log_init_file_handler(const char *filepath, long maxsize, LogFileHandler *handler);
void log_to_file(const char *file, const char *func, int line, LogFileHandler *handler, LogLevel level, uint8_t flags, const char *fmt, ...);
void log_to_stream(const char *file, const char *func, int line, FILE *stream, LogLevel level, uint8_t flags, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // LOG_H