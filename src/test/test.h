#ifdef _COMMENTS_
///# Tests
//
// The engine comes with a built-in test suite. The tests can be run with
// <code>make test</code> to verify that everything works the way it should.
// You can add your own tests by first creating a function which returns a
// <code>Result</code> according to the table below, and then adding a call to
// it in the <code>tests</code>-array in <code>test_main.cpp</code>.</br>
//
// <table class="definition-table">
// <tr><th>Result</th><th>Meaning</th></tr>
// <tr><td>PASS</td><td>The test passed and worked as expected.</td></tr>
// <tr><td>FAIL</td><td>The test failed, something didn't work as expected.</td></tr>
// <tr><td>FAIL_EXT</td><td>Something in the test's setup failed, so the test
// wasn't run.</td></tr>
// <tr><td>SKIP</td><td>The test is not yet implemented, or skipped for some
// other reason.</td></tr>
// </table></br>

//// Creating tests
//
Result math_addition() {
    return 1 + 1 == 2
        && 1 + 0 == 1
        ? PASS : FAIL;
}

//// Registering tests
//
/* test_main.cpp */
UnitTest tests[] = {
    // ...
    PASS(math_addition),
    // ...
};

//// <code>Pass</code>/<code>fail</code>
// A test can be marked either <code>PASS</code> or <code>FAIL</code>,
// depending on if it is expected to pass or fail.

#endif

namespace Test {

enum Result {
    PASS,
    FAIL,
    FAIL_EXT,
    SKIP,
};

struct UnitTest {
    const char *name;
    bool failing;

    Result (*test)();
};

#define PASS(func) { STR(func), false, func }
#define FAIL(func) { STR(func), true, func }

}  // namespace Test
