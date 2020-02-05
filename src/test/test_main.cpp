#include <random>

namespace Test {

struct UnitTest {
    const char *name;
    bool flakey;

    bool (* test)();
};

#define TEST(func) { STR(func), false, func }
#define FLAKEY(func) { STR(func), true, func }

bool math_type_size() { 
    int u = 0;
    for (int i = 0; i < 50000000; i++) {
        u += i;
    }
    return u != 0;
}
bool math_type_size_2() {
    int u = 0;
    for (int i = 0; i < 50000000; i++) {
        u += i;
    }
    return u == 0;
}

void run_tests() {
    UnitTest tests[] = {
        TEST(math_type_size),
        TEST(math_type_size_2),
        TEST(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size),
        FLAKEY(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size),
        FLAKEY(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
        FLAKEY(math_type_size_2),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
        FLAKEY(math_type_size_2),
        TEST(math_type_size_2),
        TEST(math_type_size_2),
    };
    int size = sizeof(tests) / sizeof(UnitTest);
    int passed = 0;
    int skipped = 0;
    struct timespec start, end;

    std::random_device rd;  // ensures random seed
    std::mt19937 generator(rd());
    std::shuffle(std::begin(tests), std::end(tests), generator);
    printf("Running %d tests:\n", size);

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
    double elapsed = end.tv_nsec >= start.tv_nsec
                        ? (end.tv_nsec - start.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec) * 1e3
                        : (start.tv_nsec - end.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec - 1) * 1e3;
    printf("%d out of %d tests passed (%d skipped) in %.2f ms (CPU time).\n", passed, size, skipped, elapsed);
}

}  // namespace Test
