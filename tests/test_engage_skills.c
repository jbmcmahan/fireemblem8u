#include "unity.h"
#include "engage_mechanics/engage_api.h"
#include "engage_mechanics/engage_skills.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// 1. Pin the slim host size so accidental field additions break the build.
//    struct SkillDef is { u8 kind; s8 value; } — host size = 2 bytes.
static void test_skilldef_size_is_2_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(2, sizeof(struct SkillDef));
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
        SKILL_EFFECT_PERCEPTIVE,
        SKILL_EFFECT_PERCEPTIVE_PLUS,
        SKILL_EFFECT_BREAK_DEFENSES,
        SKILL_EFFECT_UNYIELDING,
        SKILL_EFFECT_UNYIELDING_PLUS,
        SKILL_EFFECT_UNYIELDING_PLUS_PLUS,
        SKILL_EFFECT_SWORD_AGILITY,
        SKILL_EFFECT_AVOID_BONUS,
    };
    const unsigned n = sizeof(kinds) / sizeof(kinds[0]);
    for (unsigned i = 0; i < n; ++i)
        for (unsigned j = i + 1; j < n; ++j)
            TEST_ASSERT_NOT_EQUAL(kinds[i], kinds[j]);
}

// 3. Master skill registry is sized to SKILL_DEF_COUNT.
static void test_skill_def_table_size(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_DEF_COUNT * sizeof(struct SkillDef),
                           sizeof(gSkillDefs));
}

// 4. Sync unlock count is correct (count extern — array is extern without size).
static void test_sync_unlock_count_is_15(void)
{
    TEST_ASSERT_EQUAL_UINT(15, gSyncSkillUnlockCount);
}

// 5. Engage unlock table has EMBLEM_DEF_COUNT entries (count extern).
static void test_engage_unlock_count_is_12(void)
{
    TEST_ASSERT_EQUAL_UINT(EMBLEM_DEF_COUNT, gEngageSkillUnlockCount);
}

// Data-driven assertions: real tables land in engage_data.c (#85-A).
// Marth's tier-1 sync skill unlock (HP_PCT +5) is at gSyncSkillUnlocks[0];
// the .skillId field indexes into gSkillDefs[].
static void test_marth_t1_skill_kind_and_value(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gSyncSkillUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(1, gSyncSkillUnlocks[0].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_HP_PCT,
                           gSkillDefs[gSyncSkillUnlocks[0].skillId].kind);
    TEST_ASSERT_EQUAL_INT(5,
                          gSkillDefs[gSyncSkillUnlocks[0].skillId].value);
}

// Marth's tier-3 sync skill unlock resolves to a no-op slot in the
// registry (skillId = 6 was the BREAK placeholder; in #85-A's slim
// registry, that slot points at SKILL_EFFECT_NONE).
static void test_marth_t3_skill_is_none(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gSyncSkillUnlocks[1].emblemId);
    TEST_ASSERT_EQUAL_UINT(3, gSyncSkillUnlocks[1].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_NONE,
                           gSkillDefs[gSyncSkillUnlocks[1].skillId].kind);
}

// Marth's engage skill is DUAL_STRIKE (via gEngageSkillUnlocks + gSkillDefs).
static void test_marth_engage_skill_is_dual_strike(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_DUAL_STRIKE,
                           gSkillDefs[gEngageSkillUnlocks[0].skillId].kind);
}

// Resolver tests (#85-A) — exercise ApplySyncSkillsToBattleUnit,
// ApplyEngageSkillToBattleUnit, and HasInheritedSkill.

#define TEST_NO_RING 0xFF

static void test_resolver_no_ring_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = TEST_NO_RING;
    u.ringBondLevel = 15;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_bond0_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 0;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_bond15_marth_cumulative(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    // Marth t1: HP_PCT (no-op, no battleMaxHp); t3: BREAK (no-op);
    // t5: +2 ATK; t9: +5 HIT; t15: +10 AVO.
    TEST_ASSERT_EQUAL_INT(2, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(5, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(10, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_break_skill_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 3; // unlocks Marth t3 BREAK only
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_hppct_deferred_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 1; // unlocks Marth t1 HP_PCT only
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    // HP_PCT has no battle target (no battleMaxHp field); no stat changes.
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

static void test_engage_skill_idempotent(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplyEngageSkillToBattleUnit(&bu, &u);
    // Marth's engage skill is DUAL_STRIKE (no stat apply). Flag set.
    TEST_ASSERT_EQUAL_INT(1, u.uEngageSkillUsed);
    // Calling again is a no-op (idempotent).
    ApplyEngageSkillToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(1, u.uEngageSkillUsed);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

static void test_has_inherited_skill_stub(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    // gInheritSkillUnlocks is empty (#85-A), so every skillId returns false.
    TEST_ASSERT_EQUAL_INT(0, HasInheritedSkill(&u, 0));
    TEST_ASSERT_EQUAL_INT(0, HasInheritedSkill(&u, 7));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skilldef_size_is_2_bytes);
    RUN_TEST(test_skill_effect_kinds_distinct);
    RUN_TEST(test_skill_def_table_size);
    RUN_TEST(test_sync_unlock_count_is_15);
    RUN_TEST(test_engage_unlock_count_is_12);
    RUN_TEST(test_marth_t1_skill_kind_and_value);
    RUN_TEST(test_marth_t3_skill_is_none);
    RUN_TEST(test_marth_engage_skill_is_dual_strike);
    RUN_TEST(test_resolver_no_ring_noop);
    RUN_TEST(test_resolver_bond0_noop);
    RUN_TEST(test_resolver_bond15_marth_cumulative);
    RUN_TEST(test_resolver_break_skill_noop);
    RUN_TEST(test_resolver_hppct_deferred_noop);
    RUN_TEST(test_engage_skill_idempotent);
    RUN_TEST(test_has_inherited_skill_stub);
    return UNITY_END();
}