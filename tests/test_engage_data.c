#include "unity.h"
#include "engage_mechanics/engage_api.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Pin the host size so accidental field additions break the build.
static void test_emdef_size_is_40_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(40, sizeof(struct EmblemDef));
}

// 2 + 3. Dimension assertions — pure compile-time on a sized extern.
static void test_table_has_twelve_entries(void)
{
    TEST_ASSERT_EQUAL_UINT(12, sizeof(gEmblemDefs) / sizeof(gEmblemDefs[0]));
}

static void test_table_has_twelve_emdefs_size(void)
{
    TEST_ASSERT_EQUAL_UINT(12 * sizeof(struct EmblemDef), sizeof(gEmblemDefs));
}

// 6 + 7. Structural array sizes — never dereference gEmblemDefs.
static void test_sync_skills_array_size_is_five(void)
{
    TEST_ASSERT_EQUAL_UINT(5, sizeof(((struct EmblemDef *)0)->syncSkills));
}

static void test_bond_growth_stats_array_size_is_seven(void)
{
    TEST_ASSERT_EQUAL_UINT(7, sizeof(((struct EmblemDef *)0)->bondGrowthStats));
}

// 4. Every entry's name pointer non-NULL.
static void test_each_emblems_name_non_null(void)
{
    for (unsigned i = 0; i < EMBLEM_DEF_COUNT; ++i)
        TEST_ASSERT_NOT_NULL(gEmblemDefs[i].name);
}

// 5. engageWeaponItemId matches the host-side EXPECTED_IDS.
static const unsigned char EXPECTED_IDS[EMBLEM_DEF_COUNT] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

static void test_each_emblems_engageweaponid_matches_index(void)
{
    for (unsigned i = 0; i < EMBLEM_DEF_COUNT; ++i)
        TEST_ASSERT_EQUAL_UINT(EXPECTED_IDS[i], gEmblemDefs[i].engageWeaponItemId);
}

// 8. Names match canonical order.
static const char * const CANONICAL_NAMES[EMBLEM_DEF_COUNT] =
{
    "Marth",   "Celica",  "Sigurd",  "Leif",
    "Roy",     "Lyn",     "Eirika",  "Ike",
    "Micaiah", "Lucina",  "Corrin",  "Byleth",
};

static void test_emblems_names_match_canonical(void)
{
    for (unsigned i = 0; i < EMBLEM_DEF_COUNT; ++i)
        TEST_ASSERT_EQUAL_STRING(CANONICAL_NAMES[i], gEmblemDefs[i].name);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_emdef_size_is_40_bytes);
    RUN_TEST(test_table_has_twelve_entries);
    RUN_TEST(test_table_has_twelve_emdefs_size);
    RUN_TEST(test_sync_skills_array_size_is_five);
    RUN_TEST(test_bond_growth_stats_array_size_is_seven);
    RUN_TEST(test_each_emblems_name_non_null);
    RUN_TEST(test_each_emblems_engageweaponid_matches_index);
    RUN_TEST(test_emblems_names_match_canonical);
    return UNITY_END();
}
