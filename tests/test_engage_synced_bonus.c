#include "unity.h"
#include "engage_mechanics/engage_api.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Pin host size: 28 bits used, compiler pads to 4 bytes per row.
static void test_bonus_row_size_is_4_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(4, sizeof(struct BonusRow));
}

// 2. Table dimensions: 12 emblems × 20 bond levels.
static void test_synced_bonus_table_dimensions(void)
{
    TEST_ASSERT_EQUAL_UINT(EMBLEM_DEF_COUNT, sizeof(gSyncedBonuses) / sizeof(gSyncedBonuses[0]));
    TEST_ASSERT_EQUAL_UINT(20, sizeof(gSyncedBonuses[0]) / sizeof(gSyncedBonuses[0][0]));
}

// 3. Total table size = EMBLEM_DEF_COUNT * 20 * sizeof(BonusRow).
static void test_synced_bonus_table_size(void)
{
    TEST_ASSERT_EQUAL_UINT(EMBLEM_DEF_COUNT * 20 * sizeof(struct BonusRow),
                           sizeof(gSyncedBonuses));
}

// 4. Zero-init state for #85-A: every row's first nibble (hp) is 0.
static void test_synced_bonus_zero_init(void)
{
    for (unsigned e = 0; e < EMBLEM_DEF_COUNT; ++e)
        for (unsigned l = 0; l < 20; ++l)
            TEST_ASSERT_EQUAL_UINT(0, gSyncedBonuses[e][l].hp);
}

// 5. struct BonusRow is assignable and readable without aliasing.
static void test_bonus_row_round_trip(void)
{
    struct BonusRow r = { .hp = 5, .str = 3, .mag = 0,
                          .skl = 2, .spd = 4, .def = 1, .res = 7 };
    TEST_ASSERT_EQUAL_UINT(5, r.hp);
    TEST_ASSERT_EQUAL_UINT(3, r.str);
    TEST_ASSERT_EQUAL_UINT(0, r.mag);
    TEST_ASSERT_EQUAL_UINT(2, r.skl);
    TEST_ASSERT_EQUAL_UINT(4, r.spd);
    TEST_ASSERT_EQUAL_UINT(1, r.def);
    TEST_ASSERT_EQUAL_UINT(7, r.res);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_bonus_row_size_is_4_bytes);
    RUN_TEST(test_synced_bonus_table_dimensions);
    RUN_TEST(test_synced_bonus_table_size);
    RUN_TEST(test_synced_bonus_zero_init);
    RUN_TEST(test_bonus_row_round_trip);
    return UNITY_END();
}