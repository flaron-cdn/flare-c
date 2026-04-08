#ifndef FLARE_TEST_RUNNER_H
#define FLARE_TEST_RUNNER_H

#include <stdio.h>
#include <string.h>

extern int flare_tests_run;
extern int flare_tests_failed;
extern const char *flare_current_test;

void register_runtime_tests(void);
void register_request_tests(void);
void register_response_tests(void);
void register_spark_tests(void);
void register_plasma_tests(void);
void register_log_tests(void);
void register_ws_tests(void);
void register_beam_tests(void);
void register_edgeops_tests(void);

#define FLARE_TEST(name) static void name(void)

#define FLARE_RUN(name)                              \
    do {                                             \
        flare_tests_run++;                           \
        flare_current_test = #name;                  \
        int prev_failed = flare_tests_failed;        \
        name();                                      \
        if (flare_tests_failed == prev_failed) {     \
            printf("  PASS  %s\n", #name);           \
        }                                            \
    } while (0)

#define FLARE_FAIL_AT(file, line, fmt, ...)                            \
    do {                                                               \
        flare_tests_failed++;                                          \
        printf("  FAIL  %s: " fmt " (%s:%d)\n",                        \
               flare_current_test, __VA_ARGS__, file, line);           \
        return;                                                        \
    } while (0)

#define FLARE_FAIL(fmt, ...) FLARE_FAIL_AT(__FILE__, __LINE__, fmt, __VA_ARGS__)

#define FLARE_ASSERT(cond)                                         \
    do {                                                           \
        if (!(cond)) {                                             \
            FLARE_FAIL("assertion failed: %s", #cond);             \
        }                                                          \
    } while (0)

#define FLARE_ASSERT_EQ_INT(a, b)                                            \
    do {                                                                     \
        long long _a = (long long)(a);                                       \
        long long _b = (long long)(b);                                       \
        if (_a != _b) {                                                      \
            FLARE_FAIL("expected %s == %s but got %lld vs %lld", #a, #b,     \
                       _a, _b);                                              \
        }                                                                    \
    } while (0)

#define FLARE_ASSERT_EQ_STR(a, b)                                            \
    do {                                                                     \
        const char *_a = (a);                                                \
        const char *_b = (b);                                                \
        if (_a == NULL || _b == NULL || strcmp(_a, _b) != 0) {               \
            FLARE_FAIL("expected %s == %s but got '%s' vs '%s'", #a, #b,     \
                       _a ? _a : "(null)", _b ? _b : "(null)");              \
        }                                                                    \
    } while (0)

#define FLARE_ASSERT_EQ_MEM(a, len_a, b, len_b)                              \
    do {                                                                     \
        if ((len_a) != (len_b) || memcmp((a), (b), (len_a)) != 0) {          \
            FLARE_FAIL("memory differs (lens %zu vs %zu)",                   \
                       (size_t)(len_a), (size_t)(len_b));                    \
        }                                                                    \
    } while (0)

#endif
