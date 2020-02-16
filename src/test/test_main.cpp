#include <chrono>
#include <random>

#include "test.h"

#include "math.cpp"
#include "logic_callback.cpp"

#include "test_test.cpp"

namespace Test {

void run_tests() {
    UnitTest tests[] = {
        PASS(math_typedef_s8),
        PASS(math_typedef_u8),
        PASS(math_typedef_s16),
        PASS(math_typedef_u16),
        PASS(math_typedef_s32),
        PASS(math_typedef_u32),
        PASS(math_typedef_f32),
        PASS(math_typedef_s64),
        PASS(math_typedef_u64),
        PASS(math_typedef_f64),
        PASS(math_max),
        PASS(math_min),
        PASS(math_abs_max),
        PASS(math_lerp),
        PASS(math_lerp_vec2),
        PASS(math_lerp_vec3),
        PASS(math_lerp_vec4),
        PASS(math_clamp),
        PASS(math_sign_no_zero),
        PASS(math_sign),
        PASS(math_abs),
        PASS(math_mod),
        PASS(math_eq),
        PASS(math_sq),
        PASS(math_random_real),
        PASS(math_random_real_multiple),
        PASS(math_span),
        PASS(logic_callback_add_pre_update_once),
        PASS(logic_callback_add_post_update_once),
        PASS(logic_callback_add_pre_draw_once),
        PASS(logic_callback_add_post_draw_once),
        PASS(logic_callback_single_frame),
        PASS(logic_callback_forever),
        PASS(logic_callback_remove),
        PASS(logic_callback_update),

        /*
        PASS(test_slow_test_false),
        PASS(test_slow_test_false),
        PASS(test_slow_test_false),
        PASS(test_slow_test_false),
        FAIL(test_slow_test_false),
        FAIL(test_slow_test_false),
        FAIL(test_slow_test_false),
        PASS(test_slow_test_true),
        PASS(test_slow_test_true),
        PASS(test_slow_test_true),
        PASS(test_slow_test_true),
        FAIL(test_slow_test_true),
        FAIL(test_slow_test_true),
        FAIL(test_slow_test_true),
        FAIL(test_slow_test_true),
        */
    };
    int size = sizeof(tests) / sizeof(UnitTest);
    int passed = 0;  // counts only tests that should pass
    int failed = 0;  // counts only tests that should fail
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
        printf(CLEAR "\r  %d/%d: " YELLOW " testing " RESET "%s\r", current, size, test.name);
        fflush(stdout);

        Result res = test.test();
        if (res == SKIP) {
            skipped++;
            printf(CLEAR "\r" YELLOW "  skipping " RESET "%s\n", test.name);
            continue;
        }
        if (res == FAIL_EXT) {
            skipped++;
            printf("  %s fails before actual test\n", test.name);
            continue;
        }
        if (test.failing) {
            failed_expected++;
            if (res == FAIL) {
                failed++;
            } else {
                printf("  %s passed even though it shouldn't\n", test.name);
            }
        } else {
            passed_expected++;
            if (res == PASS) {
                passed++;
            } else {
                printf("  %s failed even though it shouldn't\n", test.name);
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
    printf("%d tests in %.2f ms\n", size - skipped, (elapsed.count() / 1e6));
    printf("%d illegal allocations\n", Util::num_errors);
}

}  // namespace Test
