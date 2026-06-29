#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

static void test_dummy_sanity(void)
{
    TEST_ASSERT_EQUAL_INT(2, 1 + 1);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_dummy_sanity);
    return UNITY_END();
}