#include "unity.h"
#include "engage_mechanics/engage_config.h"
#include "engage_mechanics/engage_meter.h"

void setUp(void) {}
void tearDown(void) {}

static void test_add_basic(void)
{
    TEST_ASSERT_EQUAL_UINT8(30, EngageMeter_Add(0, 30));
}

static void test_add_nonzero_current(void)
{
    TEST_ASSERT_EQUAL_UINT8(80, EngageMeter_Add(50, 30));
}

static void test_add_zero_amount(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Add(ENGAGE_METER_MAX, 0));
}

static void test_add_zero_zero(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, EngageMeter_Add(0, 0));
}

static void test_add_clamps_to_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Add(90, 30));
}

static void test_add_overflow_huge(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Add(0, 200));
}

static void test_add_at_max_plus_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Add(ENGAGE_METER_MAX, ENGAGE_METER_MAX));
}

static void test_clamp_below_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, EngageMeter_Clamp(0));
    TEST_ASSERT_EQUAL_UINT8(42, EngageMeter_Clamp(42));
}

static void test_clamp_at_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Clamp(ENGAGE_METER_MAX));
}

static void test_clamp_above_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Clamp(ENGAGE_METER_MAX + 1));
    TEST_ASSERT_EQUAL_UINT8(ENGAGE_METER_MAX, EngageMeter_Clamp(9999));
}

static void test_is_full(void)
{
    TEST_ASSERT_FALSE(EngageMeter_IsFull(0));
    TEST_ASSERT_FALSE(EngageMeter_IsFull(ENGAGE_METER_MAX - 1));
    TEST_ASSERT_TRUE(EngageMeter_IsFull(ENGAGE_METER_MAX));
}

static void test_config_flags_default_on(void)
{
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_WT_REPLACE);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_METER_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_BREAK_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_ARTS_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_CHAIN_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_ENGRAVE_ENABLED);
    TEST_ASSERT_EQUAL_INT(1, ENGAGE_INHERIT_ENABLED);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_add_basic);
    RUN_TEST(test_add_nonzero_current);
    RUN_TEST(test_add_zero_amount);
    RUN_TEST(test_add_zero_zero);
    RUN_TEST(test_add_clamps_to_max);
    RUN_TEST(test_add_overflow_huge);
    RUN_TEST(test_add_at_max_plus_max);
    RUN_TEST(test_clamp_below_max);
    RUN_TEST(test_clamp_at_max);
    RUN_TEST(test_clamp_above_max);
    RUN_TEST(test_is_full);
    RUN_TEST(test_config_flags_default_on);
    return UNITY_END();
}
