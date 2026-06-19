#include "unity.h"
#include "engage_mechanics/engage_config.h"
#include "engage_mechanics/engage_meter.h"

void setUp(void) {}
void tearDown(void) {}

static void test_add_basic(void)
{
    TEST_ASSERT_EQUAL_UINT8(30, EngageMeter_Add(0, 30));
}

static void test_add_clamps_to_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Add(90, 30));
}

static void test_clamp_overflow(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Clamp(9999));
}

static void test_is_full(void)
{
    TEST_ASSERT_TRUE(EngageMeter_IsFull(ENGAGE_METER_MAX));
    TEST_ASSERT_FALSE(EngageMeter_IsFull(ENGAGE_METER_MAX - 1));
}

static void test_config_flags_default_on(void)
{
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_METER_ENABLED);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_add_basic);
    RUN_TEST(test_add_clamps_to_max);
    RUN_TEST(test_clamp_overflow);
    RUN_TEST(test_is_full);
    RUN_TEST(test_config_flags_default_on);
    return UNITY_END();
}