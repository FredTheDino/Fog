namespace Test {

bool test_slow_test_false() {
    int a = 0;
    for (int i = 0; i < 80000000; i++) {
        a += i;
    }
    return a == 0;
}
bool test_slow_test_true() {
    int a = 0;
    for (int i = 0; i < 80000000; i++) {
        a += i;
    }
    return a != 0;
}
}
