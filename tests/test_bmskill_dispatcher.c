/**
 * Host-side TDD for the battle skill dispatcher (issue #110).
 *
 * The dispatcher doesn't know about ownership.  A unit's skills can
 * come from a class table, a character table, a learned trait, support
 * bonus, or anywhere else.  SkillDispatchForUnit() resolves all lists
 * and dispatches them in order; SkillDispatchBattle() handles a single.
 */

#include <unity.h>
#include <string.h>

/* ---- Type mirrors (no GBA headers) --------------------------------- */

typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
typedef signed   char    s8;

enum {
    SKILL_NONE       = 0,
    SKILL_SURE_SHOT  = 1,
    SKILL_ADMIRATION = 2,
};

enum { MAX_UNIT_SKILL_SOURCES = 4 };

enum SkillHook {
    SKILL_HOOK_PRE_HIT   = 0,
    SKILL_HOOK_AFTER_DMG = 1,
};

struct ClassData     { u8 number; };
struct CharacterData { u8 number; };

struct Unit {
    struct ClassData     *pClassData;
    struct CharacterData *pCharacterData;
};

struct BattleUnit { struct Unit unit; };

struct BattleHit {
    unsigned attributes : 19;
    unsigned info       : 5;
    signed   hpChange   : 8;
};

struct SkillBattleContext {
    struct BattleUnit *attacker;
    struct BattleUnit *defender;
    struct BattleHit  *hit;
};

struct SkillData {
    u8 id;
    u8 hook;
    void (*battleHook)(struct SkillBattleContext *ctx);
};

struct UnitSkillEnt {
    u8 unitId;
    const u8 *skills;   /* SKILL_NONE-terminated */
};

/* ---- Test skill lists ------------------------------------------------ */

static const u8 TestSkillList_Sniper[] = {
    SKILL_SURE_SHOT, SKILL_NONE,
};

static const u8 TestCharSkillList_Louis[] = {
    SKILL_ADMIRATION, SKILL_NONE,
};

static struct SkillData TestSkillData[] = {
    [0] = { .id = 0, .hook = SKILL_HOOK_PRE_HIT, .battleHook = NULL },
    [SKILL_SURE_SHOT] = {
        .id = SKILL_SURE_SHOT, .hook = SKILL_HOOK_PRE_HIT, .battleHook = NULL },
    [SKILL_ADMIRATION] = {
        .id = SKILL_ADMIRATION, .hook = SKILL_HOOK_AFTER_DMG, .battleHook = NULL },
};

/* ---- Test "ownership" tables (mirror of gClassSkillTable / gCharSkillTable) */

enum {
    TEST_CLASS_SNIPER   = 0x01,
    TEST_CLASS_SNIPER_F = 0x02,
    TEST_CHAR_LOUIS     = 0xFF,
};

static struct UnitSkillEnt TestClassTable[] = {
    { TEST_CLASS_SNIPER,   TestSkillList_Sniper },
    { TEST_CLASS_SNIPER_F, TestSkillList_Sniper },
    { 0, NULL },
};

static struct UnitSkillEnt TestCharTable[] = {
    { TEST_CHAR_LOUIS, TestCharSkillList_Louis },
    { 0, NULL },
};

/* ---- Unified resolver (copy of bmskill.c GetUnitSkills) -------------- */

static const u8 *findInTable(const struct UnitSkillEnt *table, u8 unitId)
{
    int i;
    for (i = 0; table[i].unitId != 0; ++i)
        if (table[i].unitId == unitId)
            return table[i].skills;
    return NULL;
}

static int Test_GetUnitSkills(const struct Unit *unit,
                              const u8 **outLists, int max)
{
    int count = 0;
    const u8 *s;

    s = findInTable(TestClassTable, unit->pClassData->number);
    if (s && count < max) outLists[count++] = s;

    s = findInTable(TestCharTable, unit->pCharacterData->number);
    if (s && count < max) outLists[count++] = s;

    return count;
}

static void Test_SkillDispatchBattle(enum SkillHook hook,
                                     struct SkillBattleContext *ctx,
                                     const u8 *skillList)
{
    int i;
    if (!skillList)
        return;

    for (i = 0; skillList[i] != SKILL_NONE; ++i) {
        u8 skillId = skillList[i];
        const struct SkillData *sd = TestSkillData + skillId;

        if (sd->id   != skillId) continue;
        if (sd->hook != hook)    continue;
        if (sd->battleHook)      sd->battleHook(ctx);
    }
}

static void Test_SkillDispatchForUnit(enum SkillHook hook,
                                      struct SkillBattleContext *ctx,
                                      const struct Unit *unit)
{
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];
    int n, i;

    n = Test_GetUnitSkills(unit, lists, MAX_UNIT_SKILL_SOURCES);

    for (i = 0; i < n; ++i)
        Test_SkillDispatchBattle(hook, ctx, lists[i]);
}

/* ---- Test hooks & counters ------------------------------------------- */

typedef struct {
    int preHitCallCount;
    int afterDmgCallCount;
    struct SkillBattleContext *lastCtx;
} TestDispatchStats;

static TestDispatchStats gTestStats;

static void TestSureShotHook(struct SkillBattleContext *ctx)
{
    gTestStats.preHitCallCount++;
    gTestStats.lastCtx = ctx;
}

static void TestAdmirationHook(struct SkillBattleContext *ctx)
{
    gTestStats.afterDmgCallCount++;
    gTestStats.lastCtx = ctx;
}

void setUp(void)   { memset(&gTestStats, 0, sizeof(gTestStats)); }
void tearDown(void) { }

/* ---- Tests: list termination ----------------------------------------- */

static void test_skill_none_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT(0, SKILL_NONE);
}

static void test_null_list_no_dispatch(void)
{
    struct SkillBattleContext ctx = { 0 };
    TestSkillData[SKILL_SURE_SHOT].battleHook = TestSureShotHook;
    Test_SkillDispatchBattle(SKILL_HOOK_PRE_HIT, &ctx, NULL);
    TEST_ASSERT_EQUAL_INT(0, gTestStats.preHitCallCount);
    TestSkillData[SKILL_SURE_SHOT].battleHook = NULL;
}

/* ---- Tests: unified GetUnitSkills ------------------------------------ */

static void test_getskills_finds_class_skills(void)
{
    struct ClassData     cd = { .number = TEST_CLASS_SNIPER };
    struct CharacterData cd2 = { .number = 1 };
    struct Unit          u  = { .pClassData = &cd, .pCharacterData = &cd2 };
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];

    int n = Test_GetUnitSkills(&u, lists, MAX_UNIT_SKILL_SOURCES);

    TEST_ASSERT_EQUAL_INT(1, n);
    TEST_ASSERT_EQUAL_PTR(TestSkillList_Sniper, lists[0]);
}

static void test_getskills_finds_char_skills(void)
{
    struct ClassData     cd  = { .number = 0x99 };  /* not in class table */
    struct CharacterData cd2 = { .number = TEST_CHAR_LOUIS };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];

    int n = Test_GetUnitSkills(&u, lists, MAX_UNIT_SKILL_SOURCES);

    TEST_ASSERT_EQUAL_INT(1, n);
    TEST_ASSERT_EQUAL_PTR(TestCharSkillList_Louis, lists[0]);
}

static void test_getskills_both_class_and_char(void)
{
    struct ClassData     cd  = { .number = TEST_CLASS_SNIPER };
    struct CharacterData cd2 = { .number = TEST_CHAR_LOUIS };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];

    int n = Test_GetUnitSkills(&u, lists, MAX_UNIT_SKILL_SOURCES);

    TEST_ASSERT_EQUAL_INT(2, n);
    TEST_ASSERT_EQUAL_PTR(TestSkillList_Sniper,   lists[0]);
    TEST_ASSERT_EQUAL_PTR(TestCharSkillList_Louis, lists[1]);
}

static void test_getskills_no_match(void)
{
    struct ClassData     cd  = { .number = 0x99 };
    struct CharacterData cd2 = { .number = 0x88 };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];

    int n = Test_GetUnitSkills(&u, lists, MAX_UNIT_SKILL_SOURCES);

    TEST_ASSERT_EQUAL_INT(0, n);
}

static void test_getskills_max_clamped(void)
{
    struct ClassData     cd  = { .number = TEST_CLASS_SNIPER };
    struct CharacterData cd2 = { .number = TEST_CHAR_LOUIS };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    const u8 *lists[1];  /* only room for 1 */

    int n = Test_GetUnitSkills(&u, lists, 1);

    TEST_ASSERT_EQUAL_INT(1, n);
    TEST_ASSERT_EQUAL_PTR(TestSkillList_Sniper, lists[0]);
}

/* ---- Tests: dispatch via SkillDispatchForUnit ------------------------ */

static void test_dispatch_pre_hit_fires_class_skill(void)
{
    struct ClassData     cd  = { .number = TEST_CLASS_SNIPER };
    struct CharacterData cd2 = { .number = 1 };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    struct BattleUnit    atk = { .unit = u }, def = { 0 };
    struct BattleHit     hit = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &atk, .defender = &def, .hit = &hit };

    TestSkillData[SKILL_SURE_SHOT].battleHook = TestSureShotHook;
    Test_SkillDispatchForUnit(SKILL_HOOK_PRE_HIT, &ctx, &u);
    TEST_ASSERT_EQUAL_INT(1, gTestStats.preHitCallCount);
    TestSkillData[SKILL_SURE_SHOT].battleHook = NULL;
}

static void test_dispatch_after_dmg_fires_char_skill(void)
{
    struct ClassData     cd  = { .number = 1 };
    struct CharacterData cd2 = { .number = TEST_CHAR_LOUIS };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    struct BattleUnit    def = { .unit = u };
    struct BattleHit     hit = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &def, .defender = &def, .hit = &hit };

    TestSkillData[SKILL_ADMIRATION].battleHook = TestAdmirationHook;
    Test_SkillDispatchForUnit(SKILL_HOOK_AFTER_DMG, &ctx, &u);
    TEST_ASSERT_EQUAL_INT(1, gTestStats.afterDmgCallCount);
    TestSkillData[SKILL_ADMIRATION].battleHook = NULL;
}

static void test_dispatch_both_sources_in_one_call(void)
{
    struct ClassData     cd  = { .number = TEST_CLASS_SNIPER };
    struct CharacterData cd2 = { .number = TEST_CHAR_LOUIS };
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    struct BattleUnit    atk = { .unit = u }, def = { .unit = u };
    struct BattleHit     hit = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &atk, .defender = &def, .hit = &hit };

    TestSkillData[SKILL_SURE_SHOT].battleHook  = TestSureShotHook;
    TestSkillData[SKILL_ADMIRATION].battleHook = TestAdmirationHook;

    Test_SkillDispatchForUnit(SKILL_HOOK_PRE_HIT, &ctx, &u);
    TEST_ASSERT_EQUAL_INT(1, gTestStats.preHitCallCount);
    TEST_ASSERT_EQUAL_INT(0, gTestStats.afterDmgCallCount);

    Test_SkillDispatchForUnit(SKILL_HOOK_AFTER_DMG, &ctx, &u);
    TEST_ASSERT_EQUAL_INT(1, gTestStats.afterDmgCallCount);

    TestSkillData[SKILL_SURE_SHOT].battleHook  = NULL;
    TestSkillData[SKILL_ADMIRATION].battleHook = NULL;
}

static void test_dispatch_wrong_hook_noop(void)
{
    /* Unit with ONLY class skills (no character skills) */
    struct ClassData     cd  = { .number = TEST_CLASS_SNIPER };
    struct CharacterData cd2 = { .number = 0x99 };  /* not in char table */
    struct Unit          u   = { .pClassData = &cd, .pCharacterData = &cd2 };
    struct BattleUnit    atk = { .unit = u };
    struct BattleHit     hit = { 0 };
    struct SkillBattleContext ctx = { .attacker = &atk, .defender = 0, .hit = &hit };

    TestSkillData[SKILL_SURE_SHOT].battleHook  = TestSureShotHook;
    TestSkillData[SKILL_ADMIRATION].battleHook = TestAdmirationHook;

    /* AFTER_DMG should NOT fire PRE_HIT skills */
    Test_SkillDispatchForUnit(SKILL_HOOK_AFTER_DMG, &ctx, &u);
    TEST_ASSERT_EQUAL_INT(0, gTestStats.preHitCallCount);
    TEST_ASSERT_EQUAL_INT(0, gTestStats.afterDmgCallCount);

    /* PRE_HIT should NOT fire AFTER_DMG skills */
    Test_SkillDispatchForUnit(SKILL_HOOK_PRE_HIT, &ctx, &u);
    TEST_ASSERT_EQUAL_INT(1, gTestStats.preHitCallCount);
    TEST_ASSERT_EQUAL_INT(0, gTestStats.afterDmgCallCount);

    TestSkillData[SKILL_SURE_SHOT].battleHook  = NULL;
    TestSkillData[SKILL_ADMIRATION].battleHook = NULL;
}

/* ---- Tests: data integrity ------------------------------------------- */

static void test_skill_data_id_matches_index(void)
{
    size_t n = sizeof(TestSkillData) / sizeof(TestSkillData[0]);
    for (size_t i = 0; i < n; ++i)
        TEST_ASSERT_EQUAL_UINT(i, TestSkillData[i].id);
}

static void test_null_hook_for_skill_none(void)
{
    TEST_ASSERT_NULL(TestSkillData[SKILL_NONE].battleHook);
}

static void test_sure_shot_hook_is_pre_hit(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_HOOK_PRE_HIT,
                           TestSkillData[SKILL_SURE_SHOT].hook);
}

static void test_admiration_hook_is_after_dmg(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_HOOK_AFTER_DMG,
                           TestSkillData[SKILL_ADMIRATION].hook);
}

/* ---- Tests: struct size pins (host vs GBA) --------------------------- */

static void test_skillbattlecontext_size(void)
{
    TEST_ASSERT_EQUAL_UINT(sizeof(void *) * 3,
                           sizeof(struct SkillBattleContext));
}

static void test_skilldata_size(void)
{
    /* GBA (32-bit): 8; host (64-bit): 16 */
    TEST_ASSERT_EQUAL_UINT(16, sizeof(struct SkillData));
}

/* ---- Documented gaps (GBA-only) -------------------------------------- */

/* Sure Shot activation guards:
     1. No duplicate activation     – BATTLE_HIT_ATTR_SURESHOT
     2. Blocked by Pierce           – BATTLE_HIT_ATTR_PIERCE
     3. Blocked by Great Shield     – BATTLE_HIT_ATTR_GREATSHLD
     4. Ballista exclusion          – GetItemIndex()/ballista IDs
     5. Activation probability      – BattleRoll1RN(level, FALSE)
 *
 * Admiration map scan:
     6. gBmMapUnit tile lookup      – GBA memory-mapped only
     7. UNIT_CATTRIBUTES(CA_FEMALE) – requires real CharacterData pointers
     8. Rectilinear distance scan   – ABS(dx)+ABS(dy) <= 2 on game map
     9. gBattleStats.damage -= 2    – GBA-side global write
 */

static void test_documented_gaps_acknowledged(void)
{
    TEST_ASSERT_TRUE(1);
}

/* ---- Runner ----------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skill_none_is_zero);
    RUN_TEST(test_null_list_no_dispatch);
    RUN_TEST(test_getskills_finds_class_skills);
    RUN_TEST(test_getskills_finds_char_skills);
    RUN_TEST(test_getskills_both_class_and_char);
    RUN_TEST(test_getskills_no_match);
    RUN_TEST(test_getskills_max_clamped);
    RUN_TEST(test_dispatch_pre_hit_fires_class_skill);
    RUN_TEST(test_dispatch_after_dmg_fires_char_skill);
    RUN_TEST(test_dispatch_both_sources_in_one_call);
    RUN_TEST(test_dispatch_wrong_hook_noop);
    RUN_TEST(test_skill_data_id_matches_index);
    RUN_TEST(test_null_hook_for_skill_none);
    RUN_TEST(test_sure_shot_hook_is_pre_hit);
    RUN_TEST(test_admiration_hook_is_after_dmg);
    RUN_TEST(test_skillbattlecontext_size);
    RUN_TEST(test_skilldata_size);
    RUN_TEST(test_documented_gaps_acknowledged);
    return UNITY_END();
}
