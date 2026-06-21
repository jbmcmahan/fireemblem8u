#include "unity.h"
#include "engage_mechanics/engage_api.h"
#include "engage_mechanics/engage_skills.h"

#include <string.h>

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

// Resolver tests (#6.2) — exercise ApplySyncSkillsToBattleUnit,
// ApplyEngageSkillToBattleUnit, and HasInheritedSkill over a
// host-portable struct Unit / struct BattleUnit pair (no GBA headers).
// We re-declare minimal stub layouts here that match the macOS mirror
// inside engage_skills.h; on macOS the host mirror is used, on GBA the
// real bmbattle.h is used by the source under test. The resolver only
// reads battle* fields + ringEmblemId/ringBondLevel/uEngageSkillUsed,
// so the offsets matter and the stubs match.

#define TEST_NO_RING 0xFF

struct TestUnit {
    unsigned char _pad_to_used[0x47];
    unsigned char uEngageSkillUsed;
    unsigned char ringEmblemId;
    unsigned char ringBondLevel;
};

struct TestBU {
    struct TestUnit unit;
    unsigned char _pad[0x5A - sizeof(struct TestUnit)];
    short battleAttack;
    unsigned char _pad2[0x60 - 0x5C];
    short battleHitRate;
    short battleAvoidRate;
    unsigned char _pad3[0x66 - 0x64];
    short battleCritRate;
};

static void test_resolver_no_ring_noop(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = TEST_NO_RING;
    u.ringBondLevel = 15;
    struct TestBU bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_bond0_noop(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 0;
    struct TestBU bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_bond15_marth_cumulative(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    struct TestBU bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    // Marth t1: HP_PCT (no-op, no battleMaxHp); t3: BREAK (no-op);
    // t5: +2 ATK; t9: +5 HIT; t15: +10 AVO.
    TEST_ASSERT_EQUAL_INT(2, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(5, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(10, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_break_skill_noop(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 3; // unlocks Marth t3 BREAK only
    struct TestBU bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
    TEST_ASSERT_EQUAL_INT(0, bu.battleHitRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAvoidRate);
    TEST_ASSERT_EQUAL_INT(0, bu.battleCritRate);
}

static void test_resolver_hppct_deferred_noop(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0;
    u.ringBondLevel = 1; // unlocks Marth t1 HP_PCT only
    struct TestBU bu; memset(&bu, 0, sizeof(bu));
    ApplySyncSkillsToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    // HP_PCT has no battle target (no battleMaxHp field); no stat changes.
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

static void test_engage_skill_idempotent(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    u.ringEmblemId = 0; // Marth
    u.ringBondLevel = 15;
    struct TestBU bu; memset(&bu, 0, sizeof(bu));
    ApplyEngageSkillToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    // Marth's engage skill is DUAL_STRIKE (no stat apply). Flag set.
    TEST_ASSERT_EQUAL_INT(1, u.uEngageSkillUsed);
    // Calling again is a no-op (idempotent).
    ApplyEngageSkillToBattleUnit((struct BattleUnit *)&bu, (struct Unit *)&u);
    TEST_ASSERT_EQUAL_INT(1, u.uEngageSkillUsed);
    TEST_ASSERT_EQUAL_INT(0, bu.battleAttack);
}

static void test_has_inherited_skill_stub(void)
{
    struct TestUnit u; memset(&u, 0, sizeof(u));
    TEST_ASSERT_EQUAL_INT(0, HasInheritedSkill((struct Unit *)&u, 0));
    TEST_ASSERT_EQUAL_INT(0, HasInheritedSkill((struct Unit *)&u, 7));
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
    RUN_TEST(test_resolver_no_ring_noop);
    RUN_TEST(test_resolver_bond0_noop);
    RUN_TEST(test_resolver_bond15_marth_cumulative);
    RUN_TEST(test_resolver_break_skill_noop);
    RUN_TEST(test_resolver_hppct_deferred_noop);
    RUN_TEST(test_engage_skill_idempotent);
    RUN_TEST(test_has_inherited_skill_stub);
    return UNITY_END();
}
