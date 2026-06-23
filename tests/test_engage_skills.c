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

// Marth's engage skill is DUAL_STRIKE (via gEngageSkillUnlocks + gSkillDefs).
static void test_marth_engage_skill_is_dual_strike(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_DUAL_STRIKE,
                           gSkillDefs[gEngageSkillUnlocks[0].skillId].kind);
}

// Resolver tests — exercise ApplySyncSkillsToBattleUnit,
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

// At bond 15, full HP, no adjacency, no sword: Unyielding and Perceptive gate
// checks prevent stat changes; only Avoid Bonus (inherit Lv 2) and Break
// Defenses flag fire.
static void test_resolver_bond15_marth_cumulative(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    u.maxHP = 30;
    u.curHP = 30; // full HP — Unyielding gate fails
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1; // non-sword — Sword Agility gate fails
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0,  bu.battleAttack);    // Unyielding: full HP
    TEST_ASSERT_EQUAL_INT(0,  bu.battleHitRate);   // Perceptive: no adjacency
    TEST_ASSERT_EQUAL_INT(10, bu.battleAvoidRate); // Avoid Bonus (inherit Lv 2)
    TEST_ASSERT_EQUAL_INT(0,  bu.battleSpeed);     // Sword Agility: non-sword
    TEST_ASSERT_EQUAL_INT(1,  gEngageBreakDefenses); // Break Defenses (bond 3)
}

// At bond 3: Break Defenses sets the flag; Avoid Bonus (inherit Lv 2)
// adds to battleAvoidRate. No other effects fire without HP/adjacency/sword.
static void test_resolver_break_skill_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 3; // unlocks Perceptive(1), Break Defenses(3) sync;
                         // Perceptive(1), Avoid+10(2) inherit
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1; // non-sword
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0,  bu.battleAttack);    // Unyielding: not unlocked yet
    TEST_ASSERT_EQUAL_INT(0,  bu.battleHitRate);   // Perceptive: no adjacency
    TEST_ASSERT_EQUAL_INT(10, bu.battleAvoidRate); // Avoid Bonus (inherit Lv 2)
    TEST_ASSERT_EQUAL_INT(0,  bu.battleCritRate);
    TEST_ASSERT_EQUAL_INT(1,  gEngageBreakDefenses);
}

static void test_resolver_hppct_deferred_noop(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 1; // Perceptive at bond 1 (sync + inherit); no other skills
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1; // non-sword
    ApplySyncSkillsToBattleUnit(&bu, &u);
    // Perceptive requires adjacency (NULL) — no stat changes.
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

// --- Unyielding family tests ---

// Bond 7 unlocks Unyielding (sync + inherit). HP <= 50% triggers +5 atk.
static void test_resolver_unyielding_applies_when_low_hp(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 7;
    u.maxHP = 30;
    u.curHP = 10; // 10*2=20 <= 30 — gate passes
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1; // non-sword
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(5, bu.battleAttack);
}

// Full HP: Unyielding gate fails, battleAttack stays 0.
static void test_resolver_unyielding_noop_when_full_hp(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 7;
    u.maxHP = 30;
    u.curHP = 30; // 30*2=60 > 30 — gate fails
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1;
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

// Bond 12 adds Unyielding+. Highest tier wins: +7 only, not +5+7=12.
static void test_resolver_unyielding_plus_suppresses_base(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 12;
    u.maxHP = 30;
    u.curHP = 10;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1;
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(7, bu.battleAttack); // Unyielding+ only, not 5+7=12
}

// --- Sword Agility tests ---

// Bond 12 unlocks Sword Agility tier 3 (inherit). Sword weapon: +3 spd.
static void test_resolver_sword_agility_applies_with_sword(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 12;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = ENGAGE_ITYPE_SWORD; // sword
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(3, bu.battleSpeed);
}

// Non-sword weapon: Sword Agility gate fails, battleSpeed stays 0.
static void test_resolver_sword_agility_noop_without_sword(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 12;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1; // lance — not a sword
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleSpeed);
}

// --- Avoid Bonus tests ---

// Bond 2 unlocks Avoid +10 (inherit). Applied unconditionally.
static void test_resolver_avoid_bonus_applies(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 2;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1;
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(10, bu.battleAvoidRate);
}

// --- Break Defenses flag tests ---

// Bond 3 unlocks Break Defenses (sync). Flag must be set.
static void test_resolver_break_defenses_sets_flag(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 3;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(1, gEngageBreakDefenses);
}

// Bond 2 does not reach Break Defenses (bond 3). Flag must stay 0.
static void test_resolver_break_defenses_noop_below_bond3(void)
{
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 2;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, gEngageBreakDefenses);
}

// --- Perceptive tests ---

static bool8 stub_adjacent_always(struct Unit *u) { (void)u; return 1; }
static bool8 stub_not_adjacent(struct Unit *u) { (void)u; return 0; }

// Bond 1 unlocks Perceptive. With an adjacent ally: +15 hit and +15 avoid.
static void test_resolver_perceptive_applies_when_adjacent_to_ally(void)
{
    Engage_AdjacentAllyCheck = stub_adjacent_always;
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 1;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1;
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(15, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(15, bu.battleAvoidRate);
    Engage_AdjacentAllyCheck = NULL;
}

// No adjacent ally: Perceptive gate fails, no hit/avoid bonus.
static void test_resolver_perceptive_noop_when_not_adjacent(void)
{
    Engage_AdjacentAllyCheck = stub_not_adjacent;
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 1;
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1;
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    Engage_AdjacentAllyCheck = NULL;
}

// Bond 16 unlocks Perceptive+. Highest tier wins: +20 only, not 15+20=35.
static void test_resolver_perceptive_plus_suppresses_base(void)
{
    Engage_AdjacentAllyCheck = stub_adjacent_always;
    struct Unit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 16;
    u.maxHP = 30;
    u.curHP = 30; // full HP — Unyielding gate fails
    struct BattleUnit bu; memset(&bu, 0, sizeof(bu));
    bu.weaponType = 1; // non-sword
    ApplySyncSkillsToBattleUnit(&bu, &u);
    TEST_ASSERT_EQUAL_INT(20, bu.battleHitRate);   // Perceptive+ only, not 15+20=35
    TEST_ASSERT_EQUAL_INT(30, bu.battleAvoidRate); // Perceptive+ (20) + Avoid Bonus (10)
    Engage_AdjacentAllyCheck = NULL;
}

// --- Engage skill and HasInheritedSkill tests ---

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
    // skillId 0 and 7 are not in gInheritSkillUnlocks.
    TEST_ASSERT_EQUAL_INT(0, HasInheritedSkill(&u, 0));
    TEST_ASSERT_EQUAL_INT(0, HasInheritedSkill(&u, 7));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skilldef_size_is_2_bytes);
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
    RUN_TEST(test_marth_engage_skill_is_dual_strike);
    RUN_TEST(test_resolver_no_ring_noop);
    RUN_TEST(test_resolver_bond0_noop);
    RUN_TEST(test_resolver_bond15_marth_cumulative);
    RUN_TEST(test_resolver_break_skill_noop);
    RUN_TEST(test_resolver_hppct_deferred_noop);
    RUN_TEST(test_resolver_unyielding_applies_when_low_hp);
    RUN_TEST(test_resolver_unyielding_noop_when_full_hp);
    RUN_TEST(test_resolver_unyielding_plus_suppresses_base);
    RUN_TEST(test_resolver_sword_agility_applies_with_sword);
    RUN_TEST(test_resolver_sword_agility_noop_without_sword);
    RUN_TEST(test_resolver_avoid_bonus_applies);
    RUN_TEST(test_resolver_break_defenses_sets_flag);
    RUN_TEST(test_resolver_break_defenses_noop_below_bond3);
    RUN_TEST(test_resolver_perceptive_applies_when_adjacent_to_ally);
    RUN_TEST(test_resolver_perceptive_noop_when_not_adjacent);
    RUN_TEST(test_resolver_perceptive_plus_suppresses_base);
    RUN_TEST(test_engage_skill_idempotent);
    RUN_TEST(test_has_inherited_skill_stub);
    return UNITY_END();
}
