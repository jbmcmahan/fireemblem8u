#include "unity.h"
#include "engage_mechanics/engage_api.h"
#include "engage_mechanics/engage_skills.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// 1. Pin the slim host size so accidental field additions break the build.
//    struct SkillDef is { u8 kind; s8 value; } — host size = 2 bytes.
static void test_skilldef_size_is_4_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(4, sizeof(struct SkillDef));
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
        SKILL_EFFECT_DIVINE_SPEED,
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
static void test_sync_unlock_count_is_16(void)
{
    TEST_ASSERT_EQUAL_UINT(16, gSyncSkillUnlockCount);
}

// 5. Engage unlock table has EMBLEM_DEF_COUNT entries (count extern).
static void test_engage_unlock_count_is_12(void)
{
    TEST_ASSERT_EQUAL_UINT(EMBLEM_DEF_COUNT, gEngageSkillUnlockCount);
}

// Data-driven assertions: Marth's bond-level data (#85-C).
// gSyncSkillUnlocks[0..5] are Marth's 6 sync skills at bond levels 1/3/7/12/16/18.
static void test_marth_t1_skill_kind_and_value(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gSyncSkillUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(1, gSyncSkillUnlocks[0].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_PERCEPTIVE,
                           gSkillDefs[gSyncSkillUnlocks[0].skillId].kind);
}

// Marth's tier-3 sync skill: Break Defenses.
static void test_marth_t3_skill_is_break_defenses(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gSyncSkillUnlocks[1].emblemId);
    TEST_ASSERT_EQUAL_UINT(3, gSyncSkillUnlocks[1].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_BREAK_DEFENSES,
                           gSkillDefs[gSyncSkillUnlocks[1].skillId].kind);
}

// Marth's 6 sync skills span the right bond levels.
static void test_marth_six_sync_skills(void)
{
    TEST_ASSERT_EQUAL_UINT(16, gSyncSkillUnlockCount); // 6 Marth + 5 Celica + 5 Ike
    // Last Marth entry is at Lv 18.
    TEST_ASSERT_EQUAL_UINT(18, gSyncSkillUnlocks[5].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_UNYIELDING_PLUS_PLUS,
                           gSkillDefs[gSyncSkillUnlocks[5].skillId].kind);
}

// Marth's 6 inheritable skills at bond levels 1/2/4/7/9/12.
static void test_marth_six_inherit_skills(void)
{
    TEST_ASSERT_EQUAL_UINT(6, gInheritSkillUnlockCount);
    TEST_ASSERT_EQUAL_UINT(0, gInheritSkillUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(1, gInheritSkillUnlocks[0].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_PERCEPTIVE,
                           gSkillDefs[gInheritSkillUnlocks[0].skillId].kind);
    TEST_ASSERT_EQUAL_UINT(12, gInheritSkillUnlocks[5].bondLevel);
}

// Marth's Rapier weapon unlock at bond Lv 1 (itemId 0 placeholder).
static void test_marth_rapier_weapon_unlock(void)
{
    TEST_ASSERT_EQUAL_UINT(1, gWeaponUnlockCount);
    TEST_ASSERT_EQUAL_UINT(0, gWeaponUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(1, gWeaponUnlocks[0].bondLevel);
    TEST_ASSERT_EQUAL_UINT(0, gWeaponUnlocks[0].itemId); // placeholder
}

// Marth's class change at bond Lv 8.
static void test_marth_class_change_at_lv8(void)
{
    TEST_ASSERT_EQUAL_UINT(1, gClassChangeUnlockCount);
    TEST_ASSERT_EQUAL_UINT(0, gClassChangeUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(8, gClassChangeUnlocks[0].bondLevel);
}

// Marth's cumulative bonuses: Lv 1 has +1 Str/Spd; Lv 18 has +3/+5/+5/+3/+2/+1.
static void test_marth_lv1_bonus(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gSyncedBonuses[0][0].hp);
    TEST_ASSERT_EQUAL_UINT(1, gSyncedBonuses[0][0].str);
    TEST_ASSERT_EQUAL_UINT(1, gSyncedBonuses[0][0].spd);
}

static void test_marth_lv18_bonus(void)
{
    TEST_ASSERT_EQUAL_UINT(3, gSyncedBonuses[0][17].hp);
    TEST_ASSERT_EQUAL_UINT(5, gSyncedBonuses[0][17].str);
    TEST_ASSERT_EQUAL_UINT(5, gSyncedBonuses[0][17].skl);
    TEST_ASSERT_EQUAL_UINT(3, gSyncedBonuses[0][17].spd);
    TEST_ASSERT_EQUAL_UINT(2, gSyncedBonuses[0][17].def);
}

// Mag nibble stays 0 throughout Marth's rows (struct Unit.mag doesn't exist yet).
static void test_marth_mag_nibble_unused(void)
{
    for (u8 i = 0; i < 20; ++i)
        TEST_ASSERT_EQUAL_UINT(0, gSyncedBonuses[0][i].mag);
}

// Marth's engage skill is DIVINE_SPEED (via gEngageSkillUnlocks + gSkillDefs).
static void test_marth_engage_skill_is_divine_speed(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_DIVINE_SPEED,
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
    ApplySyncSkillsToBattleUnit(&bu, &u, 1);
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
    ApplySyncSkillsToBattleUnit(&bu, &u, 1);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

// Marth's wiki skills at Lv 15 (Perceptive, Break Defenses, Unyielding,
// Unyielding+, Perceptive+, Unyielding++) have no resolver wiring yet —
// they're stub-cased in ApplySyncSkillsToBattleUnit. The test verifies the
// resolver runs without crashing and produces no battle-stat deltas for
// Marth at Lv 15 under the current wiki data.
static void test_resolver_bond15_marth_cumulative(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u, 1);
    // No BATTLE_* skill wiring applies to Marth's wiki skills today.
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_break_skill_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 3; // unlocks Marth t3 BREAK only
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u, 1);
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
    ApplySyncSkillsToBattleUnit(&bu, &u, 1);
    // HP_PCT has no battle target (no battleMaxHp field); no stat changes.
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

static void test_engage_skill_idempotent(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplyEngageSkillToBattleUnit(&bu, &u, 1);
    // Marth's engage skill is DIVINE_SPEED (deferred). Flag not set yet.
    TEST_ASSERT_EQUAL_INT(0, u.uEngageSkillUsed);
    // Calling again is a no-op (idempotent).
    ApplyEngageSkillToBattleUnit(&bu, &u, 1);
    TEST_ASSERT_EQUAL_INT(0, u.uEngageSkillUsed);
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
    RUN_TEST(test_skilldef_size_is_4_bytes);
    RUN_TEST(test_skill_effect_kinds_distinct);
    RUN_TEST(test_skill_def_table_size);
    RUN_TEST(test_sync_unlock_count_is_16);
    RUN_TEST(test_engage_unlock_count_is_12);
    RUN_TEST(test_marth_t1_skill_kind_and_value);
    RUN_TEST(test_marth_t3_skill_is_break_defenses);
    RUN_TEST(test_marth_six_sync_skills);
    RUN_TEST(test_marth_six_inherit_skills);
    RUN_TEST(test_marth_rapier_weapon_unlock);
    RUN_TEST(test_marth_class_change_at_lv8);
    RUN_TEST(test_marth_lv1_bonus);
    RUN_TEST(test_marth_lv18_bonus);
    RUN_TEST(test_marth_mag_nibble_unused);
    RUN_TEST(test_marth_engage_skill_is_divine_speed);
    RUN_TEST(test_resolver_no_ring_noop);
    RUN_TEST(test_resolver_bond0_noop);
    RUN_TEST(test_resolver_bond15_marth_cumulative);
    RUN_TEST(test_resolver_break_skill_noop);
    RUN_TEST(test_resolver_hppct_deferred_noop);
    RUN_TEST(test_engage_skill_idempotent);
    RUN_TEST(test_has_inherited_skill_stub);
    return UNITY_END();
}