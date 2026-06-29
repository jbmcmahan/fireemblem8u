#include <string.h>
#include <unity.h>

typedef unsigned char u8;
typedef signed char s8;

#define TRUE 1
#define FALSE 0

enum {
    SKILL_NONE = 0,
    SKILL_CONTEMPLATIVE = 9,
};

struct Unit {
    u8 index;
    u8 skill;
};

struct BattleUnit {
    struct Unit unit;
    int battleDefense;
};

static u8 sUnitContemplativeActive[0x100];

static s8 UnitHasSkill(const struct Unit *unit, u8 skill)
{
    return unit && unit->skill == skill;
}

static void SkillClearUnitCombatState(const struct Unit *unit)
{
    if (!unit || !unit->index)
        return;

    sUnitContemplativeActive[unit->index] = FALSE;
}

static void SkillOnUnitBeginAction(const struct Unit *unit)
{
    if (!unit || !unit->index)
        return;

    sUnitContemplativeActive[unit->index] = FALSE;
}

static void SkillOnUnitWait(const struct Unit *unit)
{
    if (!unit || !unit->index)
        return;

    if (!UnitHasSkill(unit, SKILL_CONTEMPLATIVE))
        return;

    sUnitContemplativeActive[unit->index] = TRUE;
}

static s8 SkillUnitHasContemplativeBonus(const struct Unit *unit)
{
    if (!unit || !unit->index)
        return FALSE;

    return sUnitContemplativeActive[unit->index];
}

static void ApplyContemplative(struct BattleUnit *bu)
{
    if (SkillUnitHasContemplativeBonus(&bu->unit))
        bu->battleDefense += 2;
}

void setUp(void)
{
    memset(sUnitContemplativeActive, 0, sizeof(sUnitContemplativeActive));
}

void tearDown(void) {}

static void test_wait_activates_contemplative_bonus(void)
{
    struct Unit unit = { .index = 1, .skill = SKILL_CONTEMPLATIVE };
    struct BattleUnit bu = { .unit = unit, .battleDefense = 8 };

    SkillOnUnitWait(&unit);
    ApplyContemplative(&bu);

    TEST_ASSERT_EQUAL_INT(10, bu.battleDefense);
}

static void test_nonowners_do_not_gain_bonus_from_wait(void)
{
    struct Unit unit = { .index = 1, .skill = SKILL_NONE };
    struct BattleUnit bu = { .unit = unit, .battleDefense = 8 };

    SkillOnUnitWait(&unit);
    ApplyContemplative(&bu);

    TEST_ASSERT_EQUAL_INT(8, bu.battleDefense);
}

static void test_bonus_clears_when_unit_begins_next_action(void)
{
    struct Unit unit = { .index = 1, .skill = SKILL_CONTEMPLATIVE };
    struct BattleUnit bu = { .unit = unit, .battleDefense = 8 };

    SkillOnUnitWait(&unit);
    SkillOnUnitBeginAction(&unit);
    ApplyContemplative(&bu);

    TEST_ASSERT_EQUAL_INT(8, bu.battleDefense);
}

static void test_bonus_clears_when_unit_slot_is_reset(void)
{
    struct Unit unit = { .index = 1, .skill = SKILL_CONTEMPLATIVE };
    struct BattleUnit bu = { .unit = unit, .battleDefense = 8 };

    SkillOnUnitWait(&unit);
    SkillClearUnitCombatState(&unit);
    ApplyContemplative(&bu);

    TEST_ASSERT_EQUAL_INT(8, bu.battleDefense);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_wait_activates_contemplative_bonus);
    RUN_TEST(test_nonowners_do_not_gain_bonus_from_wait);
    RUN_TEST(test_bonus_clears_when_unit_begins_next_action);
    RUN_TEST(test_bonus_clears_when_unit_slot_is_reset);
    return UNITY_END();
}
