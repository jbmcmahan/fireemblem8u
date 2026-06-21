#include "unity.h"
#include "engage_mechanics/engage_api.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Pin the host size so accidental field additions break the build.
static void test_skilldef_size_is_8_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(8, sizeof(struct SkillDef));
}

// 2. Every effect kind is distinct so no accidental aliasing.
static void test_skill_effect_kinds_distinct(void)
{
    const unsigned kinds[] = {
        SKILL_EFFECT_NONE,
        SKILL_EFFECT_HP_PCT,
        SKILL_EFFECT_BATTLE_ATK,
        SKILL_EFFECT_BATTLE_HIT,
        SKILL_EFFECT_BATTLE_AVO,
        SKILL_EFFECT_BATTLE_CRIT,
        SKILL_EFFECT_BREAK,
        SKILL_EFFECT_DUAL_STRIKE,
    };
    const unsigned n = sizeof(kinds) / sizeof(kinds[0]);
    for (unsigned i = 0; i < n; ++i)
        for (unsigned j = i + 1; j < n; ++j)
            TEST_ASSERT_NOT_EQUAL(kinds[i], kinds[j]);
}

// 3 + 4. Pure compile-time size assertions on the sized externs.
static void test_sync_table_size(void)
{
    TEST_ASSERT_EQUAL_UINT(12 * SKILL_DEF_COUNT_SYNC,
                           sizeof(gSyncSkillDefs) / sizeof(gSyncSkillDefs[0]));
    TEST_ASSERT_EQUAL_UINT(12 * SKILL_DEF_COUNT_SYNC * sizeof(struct SkillDef),
                           sizeof(gSyncSkillDefs));
}

static void test_engage_table_size(void)
{
    TEST_ASSERT_EQUAL_UINT(12 * SKILL_DEF_COUNT_ENGAGE,
                           sizeof(gEngageSkillDefs) / sizeof(gEngageSkillDefs[0]));
    TEST_ASSERT_EQUAL_UINT(12 * SKILL_DEF_COUNT_ENGAGE * sizeof(struct SkillDef),
                           sizeof(gEngageSkillDefs));
}

// Data-driven assertions: real tables land in engage_data.c (#6.1).
static void test_marth_hp5_skill_kind_and_value(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_HP_PCT, gSyncSkillDefs[0 * 5 + 0].kind);
    TEST_ASSERT_EQUAL_INT(5, gSyncSkillDefs[0 * 5 + 0].value);
}

static void test_marth_break_skill_at_tier_3(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_BREAK, gSyncSkillDefs[0 * 5 + 1].kind);
    TEST_ASSERT_EQUAL_UINT(3, gSyncSkillDefs[0 * 5 + 1].tier);
}

static void test_marth_engage_skill_kind_is_dual_strike(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_DUAL_STRIKE, gEngageSkillDefs[0].kind);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skilldef_size_is_8_bytes);
    RUN_TEST(test_skill_effect_kinds_distinct);
    RUN_TEST(test_sync_table_size);
    RUN_TEST(test_engage_table_size);
    RUN_TEST(test_marth_hp5_skill_kind_and_value);
    RUN_TEST(test_marth_break_skill_at_tier_3);
    RUN_TEST(test_marth_engage_skill_kind_is_dual_strike);
    return UNITY_END();
}
