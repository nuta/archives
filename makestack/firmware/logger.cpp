#include <makestack/types.h>
#include <makestack/logger.h>

static char *ring_buf;
static int read_p = 0;
static int write_p = 0;

void init_logger() {
    ring_buf = (char *) malloc(LOGGER_BUF_SIZE);
    if (!ring_buf) {
        printf("WARN: failed to allocate the logger buffer!\n");
    }
}

void logger(const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);
    vlogger(format, vargs);
    va_end(vargs);
}

void vlogger(const char *format, va_list vargs) {
    char buf[256];
    size_t str_len = vsnprintf(buf, sizeof(buf), format, vargs);
    printf("%s", (char *) &buf);

    size_t copy_len1 = min(str_len, LOGGER_BUF_SIZE - write_p);
    memcpy(ring_buf + write_p, buf, copy_len1);

    str_len -= copy_len1;
    if (str_len > 0) {
        // Wrapping.
        size_t copy_len2 = min(str_len, LOGGER_BUF_SIZE);
        memcpy(ring_buf, buf + copy_len1, copy_len2);
        write_p = copy_len2;
    } else {
        write_p += copy_len1;
    }
}

char *read_logger_buffer(size_t *length) {
    *length = (read_p < write_p) ? write_p - read_p : LOGGER_BUF_SIZE - read_p;
    char *p = ring_buf + read_p;
    read_p = write_p;
    return p;
}
