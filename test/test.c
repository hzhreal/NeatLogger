#include "log.h"

int main(void) {
    const char *string = "Hello World!";

    LogFileHandler handler;
    log_init_file_handler("test.log", LOG_DEFAULT_MAX_FILESIZE, &handler);
    LOG_TO_FILE(&handler, LOG_LEVEL_INFO, LOG_ALL_FILE, "%s", string);
    LOG_TO_STREAM(stdout, LOG_LEVEL_INFO, LOG_ALL, "%s", string);
    log_kill_file_handler(&handler);

    return 0;
}