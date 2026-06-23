#include "unity.h"
#include "engage_mechanics/engage_api.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Pin the host size so accidental field additions break the build.
//    struct SkillDef is { u8 kind; s8 value; u8 group; u8 _pad; } — host size = 4 bytes.
static void test_skilldef_size_is_4_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(4, sizeof(struct SkillDef));
}

// 2. Sentinel slot is the SKILL_EFFECT_NONE row.
static void test_skilldef_index_zero_is_none(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_NONE, gSkillDefs[0].kind);
    TEST_ASSERT_EQUAL_INT(0, gSkillDefs[0].value);
}

// 3. Registry has room for growth (>= 64 entries).
static void test_skill_def_count_at_least_64(void)
{
    TEST_ASSERT(SKILL_DEF_COUNT >= 64);
}

// 4. Sized extern matches count × sizeof so the table can't drift.
static void test_skill_def_table_size(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_DEF_COUNT * sizeof(struct SkillDef),
                           sizeof(gSkillDefs));
}

// 5. New Marth-specific skill enum entries land in the registry's
//    indices 8..15 (one row per kind).
static void test_new_marth_skill_names_present(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_PERCEPTIVE,
                           gSkillDefs[SKILL_EFFECT_PERCEPTIVE].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_PERCEPTIVE_PLUS,
                           gSkillDefs[SKILL_EFFECT_PERCEPTIVE_PLUS].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_BREAK_DEFENSES,
                           gSkillDefs[SKILL_EFFECT_BREAK_DEFENSES].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_UNYIELDING,
                           gSkillDefs[SKILL_EFFECT_UNYIELDING].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_UNYIELDING_PLUS,
                           gSkillDefs[SKILL_EFFECT_UNYIELDING_PLUS].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_UNYIELDING_PLUS_PLUS,
                           gSkillDefs[SKILL_EFFECT_UNYIELDING_PLUS_PLUS].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_SWORD_AGILITY,
                           gSkillDefs[SKILL_EFFECT_SWORD_AGILITY].kind);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_AVOID_BONUS,
                           gSkillDefs[SKILL_EFFECT_AVOID_BONUS].kind);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skilldef_size_is_4_bytes);
    RUN_TEST(test_skilldef_index_zero_is_none);
    RUN_TEST(test_skill_def_count_at_least_64);
    RUN_TEST(test_skill_def_table_size);
    RUN_TEST(test_new_marth_skill_names_present);
    return UNITY_END();
}