#include <chrono>
#include <random>
#include <vector>

#include "test.h"

#include "math.cpp"
#include "logic_callback.cpp"

namespace Test {

void run_tests() {
    std::vector<UnitTest> tests;

    init_math(&tests);
    init_logic_callback(&tests);

    int passed = 0;  // counts only tests that should pass, not unexpected passes
    int failed = 0;  // counts only tests that should fail, not unexpected fails
    int passed_expected = 0;
    int failed_expected = 0;
    int skipped = 0;

    // std::random_device rd;
    // int seed = rd();
    // printf("Shuffling with seed %d", seed);
    // std::mt19937 generator(seed);
    // std::shuffle(std::begin(tests), std::end(tests), generator);

    auto start = std::chrono::steady_clock::now();
    int current = 0;
    for (UnitTest test: tests) {
        current++;
        printf(CLEAR "\r%d/%d: " YELLOW " testing " RESET "%s\r", current, tests.size(), test.name);
        fflush(stdout);

        Result res = test.test();
        if (res == SKIP) {
            skipped++;
            printf(CLEAR "\r" YELLOW "s " RESET "%s skipped\n", test.name);
            continue;
        }
        if (res == FAIL_EXT) {
            skipped++;
            printf(CLEAR "\r" YELLOW "s " RESET "%s fails before actual test\n", test.name);
            continue;
        }
        if (test.failing) {
            failed_expected++;
            if (res == FAIL) {
                failed++;
            } else {
                printf(CLEAR "\r" RED "p " RESET "%s passed even though it shouldn't\n", test.name);
            }
        } else {
            passed_expected++;
            if (res == PASS) {
                passed++;
            } else {
                printf(CLEAR "\r" RED "f " RESET "%s failed even though it shouldn't\n", test.name);
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::nanoseconds elapsed = end - start;

    printf(CLEAR);
    printf(GREEN  "passed:" RESET "               %3d\n", passed);
    printf(RED    "unexpected failures:" RESET "  %3d\n", (passed_expected - passed));
    printf(RED    "unexpected successes:" RESET " %3d\n", (failed_expected - failed));
    printf(YELLOW "expected failures:" RESET "    %3d\n", failed);
    printf(YELLOW "skipped:" RESET "              %3d\n", skipped);
    printf("%d tests in %.2f ms\n", tests.size() - skipped, (elapsed.count() / 1e6));
    printf("%d illegal allocations\n", Util::num_errors);
}

}  // namespace Test
