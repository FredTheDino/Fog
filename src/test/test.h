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
    bool skip;

    Result (*test)();
};

#define PASS(func) { STR(func), false, false, func }
#define FAIL(func) { STR(func), true, false, func }
#define SKIP(func) { STR(func), true, true, func }
}
