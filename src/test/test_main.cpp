#include <random>

#include "math.cpp"

namespace Test {

struct UnitTest {
    const char *name;
    bool flakey;

    bool (* test)();
};

#define TEST(func) { STR(func), false, func }
#define FLAKEY(func) { STR(func), true, func }

void run_tests() {
    UnitTest tests[] = {
        TEST(math_typedef_s8),
        TEST(math_typedef_u8),
        TEST(math_typedef_s16),
        TEST(math_typedef_u16),
        TEST(math_typedef_s32),
        TEST(math_typedef_u32),
        TEST(math_typedef_f32),
        TEST(math_typedef_s64),
        TEST(math_typedef_u64),
        TEST(math_typedef_f64),
    };
    int size = sizeof(tests) / sizeof(UnitTest);
    int passed = 0;
    int skipped = 0;
    struct timespec start, end;

    std::random_device rd;  // ensures random seed
    std::mt19937 generator(rd());
    std::shuffle(std::begin(tests), std::end(tests), generator);
    printf("Running %d tests\n", size);

    printf(HIDE_CURSOR);
    clock_gettime(CLOCK_MONOTONIC, &start);
    int current = 0;
    for (UnitTest test: tests) {
        current++;
        if (test.flakey) {
            skipped++;
            printf(CLEAR "\r" YELLOW "    skipping " RESET "%s\n", test.name);
            continue;
        }
        printf("    %d/%d: " YELLOW " testing " RESET "%s\r", current, size, test.name);
        fflush(stdout);
        if (test.test()) {
            passed++;
        } else {
            printf(CLEAR "\r" RED "    failed " RESET "%s\n", test.name);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    // https://stackoverflow.com/a/49235959
    double elapsed = end.tv_nsec >= start.tv_nsec
                        ? (end.tv_nsec - start.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec) * 1e3
                        : (start.tv_nsec - end.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec - 1) * 1e3;
    float percent = 100.0 * passed / (size - skipped);
    if (percent >= 99.9) printf(GREEN);  // close enough
    else printf(YELLOW);
    printf("%d out of %d tests" RESET " (%.0f%%) passed (%d skipped) in %.2f ms (CPU time).\n", passed, size, (100.0 * passed / size), skipped, elapsed);
    printf(SHOW_CURSOR);
}

}  // namespace Test
