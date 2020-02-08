#include <random>

#include "math.cpp"
#include "test_test.cpp"

namespace Test {

struct UnitTest {
    const char *name;
    bool failing;
    bool skip;

    bool (* test)();
};

#define PASSING(func) { STR(func), false, false, func }
#define FAILING(func) { STR(func), true, false, func }
#define SKIP(func)    { STR(func), true, true, func }

void run_tests() {
    UnitTest tests[] = {
        PASSING(math_typedef_s8),
        PASSING(math_typedef_u8),
        PASSING(math_typedef_s16),
        PASSING(math_typedef_u16),
        PASSING(math_typedef_s32),
        PASSING(math_typedef_u32),
        PASSING(math_typedef_f32),
        PASSING(math_typedef_s64),
        PASSING(math_typedef_u64),
        PASSING(math_typedef_f64),
        PASSING(test_slow_test_false),
        PASSING(test_slow_test_false),
        PASSING(test_slow_test_false),
        PASSING(test_slow_test_false),
        FAILING(test_slow_test_false),
        FAILING(test_slow_test_false),
        FAILING(test_slow_test_false),
        PASSING(test_slow_test_true),
        PASSING(test_slow_test_true),
        PASSING(test_slow_test_true),
        PASSING(test_slow_test_true),
        FAILING(test_slow_test_true),
        FAILING(test_slow_test_true),
        FAILING(test_slow_test_true),
        FAILING(test_slow_test_true),
    };
    int size = sizeof(tests) / sizeof(UnitTest);
    int passed = 0;  // counts only tests that should pass
    int failed = 0;  // counts only tests that should fail
    int passed_expected = 0;
    int failed_expected = 0;
    int skipped = 0;
    struct timespec start, end;

    // std::random_device rd;  // ensures random seed
    // std::mt19937 generator(rd());
    // std::shuffle(std::begin(tests), std::end(tests), generator);

    clock_gettime(CLOCK_MONOTONIC, &start);
    int current = 0;
    for (UnitTest test: tests) {
        current++;
        if (test.skip) {
            skipped++;
            printf(CLEAR "\r" YELLOW "    skipping " RESET "%s\n", test.name);
            continue;
        }
        printf(CLEAR "\r    %d/%d: " YELLOW " testing " RESET "%s\r", current, size, test.name);
        fflush(stdout);

        if (test.failing) {
            failed_expected++;
            if (!test.test()) {
                failed++;
            } else {
                printf("    %s passed even though it shouldn't\n", test.name);
            }
        } else {
            passed_expected++;
            if (test.test()) {
                passed++;
            } else {
                printf("    %s failed even though it shouldn't\n", test.name);
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    // https://stackoverflow.com/a/49235959
    double elapsed = end.tv_nsec >= start.tv_nsec
                        ? (end.tv_nsec - start.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec) * 1e3
                        : (start.tv_nsec - end.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec - 1) * 1e3;
    float percent = 100.0 * passed / (size - skipped);

    printf(CLEAR);
    printf(GREEN  "passed:" RESET "                %d\n", passed);
    printf(RED    "failed:" RESET "                %d\n", failed);
    printf(RED    "unexpected successes:" RESET "  %d\n", (failed_expected - failed));
    printf(YELLOW "expected failures:" RESET "     %d\n", failed);
    printf(YELLOW "skipped:" RESET "               %d\n", skipped);
    printf("%d tests in %.2f ms\n", size, elapsed);
}

}  // namespace Test
