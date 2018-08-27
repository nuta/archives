#include "api.h"
#include "malloc.h"
#include "string.h"

static int tests_failed = 0;

#define ASSERT_EQ(expr1, expr2) \
    do { \
        if ((expr1) == (expr2)) { \
            fprintf(stderr, "."); \
            fflush(stderr); \
        } else { \
            DEBUG("\nFAIL:%s:%d: expected %s == %s", __func__, __LINE__, #expr1, #expr2); \
            tests_failed++; \
            return; \
        } \
    } while (0)

#define ASSERT_NOT_EQ(expr1, expr2) \
    do { \
        if ((expr1) != (expr2)) { \
            fprintf(stderr, "."); \
            fflush(stderr); \
        } else { \
            DEBUG("\nFAIL:%s:%d: expected %s != %s", __func__, __LINE__, #expr1, #expr2); \
            tests_failed++; \
            return; \
        } \
    } while (0)

#define UNITTEST(name) static void test_ ## name (void)

UNITTEST(internal) {
    ASSERT_EQ(ena_strlen(""), 0);
    ASSERT_EQ(ena_strlen("abc"), 3);
    ASSERT_EQ(ena_strcmp("", ""), 0);
    ASSERT_NOT_EQ(ena_strcmp("abc", ""), 0);
    ASSERT_EQ(ena_strcmp("abc", "abc"), 0);
    ASSERT_NOT_EQ(ena_strcmp("abcdefg", "abc"), 0);
    ASSERT_NOT_EQ(ena_strncmp("abc", "abC", 3), 0);
    ASSERT_NOT_EQ(ena_strncmp("abc", "abC", 3), 0);

    {
        char *duped_str = ena_strdup("create_file");
        ASSERT_EQ(ena_strcmp(duped_str, "create_file"), 0);
        ena_free(duped_str);
    }

    {
        char *duped_str = ena_strndup("create_file", 5);
        ASSERT_EQ(ena_strcmp(duped_str, "creat"), 0);
        ena_free(duped_str);
    }

    ASSERT_EQ(ena_str2int("0"), 0);
    ASSERT_EQ(ena_str2int("1"), 1);
    ASSERT_EQ(ena_str2int("789012345"), 789012345);
    ASSERT_EQ(ena_str2int("0x123"), 0x123);
    ASSERT_EQ(ena_str2int("0x1a1"), 0x1a1);
    ASSERT_EQ(ena_str2int("0x000"), 0x0);

    ASSERT_EQ(ena_memcmp("a\0bc", "vwxyz", 0), 0);
    ASSERT_EQ(ena_memcmp("a\0bc", "a\0bc", 4), 0);
    ASSERT_EQ(ena_memcmp("a\0bc", "a\0bC", 3), 0);
    ASSERT_NOT_EQ(ena_memcmp("a\0bc", "a\0bC", 4), 0);
    ASSERT_NOT_EQ(ena_memcmp("a\0bc", "a\0Bc", 4), 0);
    {
        char buf[4];
        ena_memcpy(&buf, "a\0bc", sizeof(buf));
        ASSERT_EQ(ena_memcmp(&buf, "a\0bc", sizeof(buf)), 0);
    }
}

UNITTEST(ident) {
    struct ena_vm *vm = ena_create_vm();
    const char *str1 = "This is a string!";

    ena_ident_t ident1 = ena_cstr2ident(vm, str1);
    ASSERT_EQ(ident1, ena_cstr2ident(vm, str1));

    const char *str2 = ena_ident2cstr(vm, ident1);
    ASSERT_NOT_EQ(str2, NULL);
    ASSERT_EQ(ena_strcmp(str1, str2), 0);
}

// ena_strlen() used for computing the length of a UTF-8 byte sequence.
#define UTF8_TEST_EQ(func, string, expected, ...) \
    do { \
        const char *str = string; \
        ASSERT_EQ(func(str, ena_strlen(str), ##__VA_ARGS__), expected); \
    } while (0)

UNITTEST(string) {
    UTF8_TEST_EQ( utf8_validate, u8"", true);
    UTF8_TEST_EQ(utf8_validate, u8"This is an ASCII string.", true);
    UTF8_TEST_EQ(utf8_strlen, u8"", 0);
    UTF8_TEST_EQ(utf8_strlen, u8"Hello World!", 12);
    UTF8_TEST_EQ(utf8_strlen, u8"すき焼き", 4);
    UTF8_TEST_EQ(utf8_strlen, u8"I wanna go to a 🍣 bar.", 22);
    UTF8_TEST_EQ(utf8_strlen, u8"あらゆる現実を全て自分の方へねじ曲げたのだ", 21);
    UTF8_TEST_EQ(utf8_char_at, u8"A 🍔.", 0x1f354, 2);
    UTF8_TEST_EQ(utf8_char_at, u8"Enaga is 鳥.", 0x9ce5, 9);
    UTF8_TEST_EQ(utf8_char_at, u8"Petit déjeuner.", 0xe9, 7);
}

UNITTEST(hash) {
    struct ena_hash_table table;
    ena_hash_init_ident_table(&table);
    ena_hash_insert(&table, (void *) 0, "string1");
    ena_hash_insert(&table, (void *) 1, "string3");
    ena_hash_insert(&table, (void *) INITIAL_NUM_BUCKETS, "string2");
    ena_hash_insert(&table, (void *) (INITIAL_NUM_BUCKETS * 2), "string3");
    ena_hash_insert(&table, (void *) (INITIAL_NUM_BUCKETS * 3), "string4");
}

int ena_unittests(void) {
    fprintf(stderr, "unit tests");
    fflush(stderr);

    test_internal();
    test_ident();
    test_string();
    test_hash();

    if (tests_failed) {
        DEBUG("\n### %d tests failed ###", tests_failed);
    } else {
        DEBUG("ok");
    }
    return tests_failed;
}
