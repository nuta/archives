#include <efijs/types.h>

extern "C" {

#include <efijs/message.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <efi.h>
#include <efilib.h>

FILE * const stdin  = 0;
FILE * const stdout = 0;
FILE * const stderr = 0;

#pragma GCC diagnostic ignored "-Wunused-parameter"

void printchar(const char ch) {

    Print((const CHAR16 *) L"%c", ch);
}

void *malloc(size_t size) {

    void *ptr = AllocatePool(size);
    printf("malloc %d bytes at 0x%x", size, ptr);
    return ptr;
}

void free(void *ptr) {

    FreePool(ptr);
}

void *realloc(void *ptr, size_t size) {

    EFIJS_NYI();
}

void *calloc(size_t size, size_t n) {

    void *p = malloc(size * n);
    memset(p, 0, size * n);

    return p;
}

FILE* fopen(const char* filename, const char* mode) {

    EFIJS_NYI();
    return NULL;
}

int fflush(FILE* f) {

    EFIJS_NYI();
}

int fclose(FILE* f) {

    EFIJS_NYI();
}

int fseeko(FILE* stream, off_t offset, int whence) {

    EFIJS_NYI();
}

off_t ftello(FILE* stream) {

    EFIJS_NYI();
}

int fseek(FILE* f, long off, int whence) {

    EFIJS_NYI();
}

long ftell(FILE* f) {

    EFIJS_NYI();
}

char* fgets(char* s, int n, FILE* f) {

    EFIJS_NYI();
}


int fputc(int c, FILE* f) {

    EFIJS_NYI();
}

size_t fread(void* destv, size_t size, size_t nmemb, FILE* f) {

    EFIJS_NYI();
}

int feof(FILE* f) {

    EFIJS_NYI();
}

int ferror(FILE* f) {

    EFIJS_NYI();
}

void rewind(FILE* f) {

    EFIJS_NYI();
}

int setvbuf(FILE* f, char* buf, int type, size_t size) {

    EFIJS_NYI();
}

void sched_yield() {

    EFIJS_NYI();
}

void exit(int exit_code) {

    EFIJS_NYI();
}

void abort() {

    EFIJS_NYI();
}

void __assert_fail(const char *expr, const char* file, int line, const char* func) {

    EFIJS_NYI();
}


int sprintf(char *s, const char *fmt, ...) {

    EFIJS_NYI();
    return 0;
}

int snprintf(char *s, size_t n, const char *fmt, ...) {

    EFIJS_NYI();
    return 0;
}

int asprintf(char **s, const char *fmt, ...) {

    EFIJS_NYI();
    return 0;
}

int fprintf(FILE *f, const char *fmt, ...) {

    EFIJS_NYI();
    return 0;
}

int swprintf(wchar_t* s, size_t n, const wchar_t* fmt, ...) {

    EFIJS_NYI();
}

int sscanf(const char* s, const char* fmt, ...) {

    EFIJS_NYI();
}

int vsscanf(const char* s, const char* fmt, va_list ap) {

    EFIJS_NYI();
}

int* __errno_location(void) {
    static int errno = 0;

    return &errno;
}

int __uflow(FILE* f) {

    EFIJS_NYI();
}

int fesetround() {

    EFIJS_NYI();
    return 0;
}

int pthread_self() {

    EFIJS_NYI();
    return 0;
}


size_t strftime_l(char* s, size_t n, const char* f, const struct tm* tm, locale_t l) {

    EFIJS_NYI();
}


const struct __locale_map *__get_locale(int cat, const char *val) {
    EFIJS_IGNORED_NYI();
    return NULL;
}


char *setlocale(int cat, const char *name) {
    EFIJS_IGNORED_NYI();
    return "C";
}

} // extern "C"
