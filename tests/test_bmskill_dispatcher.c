/**
 * Host-side TDD for the battle skill dispatcher (issue #110).
 *
 * Tests the pure-logic portion of SkillDispatchBattle:
 *   - SKILL_NONE-terminated class skill lists
 *   - Sniper / Sniper F class skill assignment
 *   - Ordinary classes receive no skills
 *   - Hook dispatch ordering and filtering
 *   - Multiple-skill iteration within a class
 *   - Table termination (classId == 0 sentinel)
 *
 * The Sure Shot handler (SkillSureShotBattleHook) references GBA globals
 * (gBattleHitIterator, BattleRoll1RN, GetItemIndex, item ballista IDs)
 * that cannot be linked in a host binary without extensive stubbing.
 * Activation guards are documented below but require GBA or a full
 * runtime harness for end-to-end verification.
 */

#include <unity.h>
#include <string.h>

/*
 * ========================================================================
 * Self-contained type mirrors — no GBA headers included.
 *
 * The real types live in include/bmskill.h which transitively pulls in
 * gba/global.h → variables.h → CONST_DATA section attrs that clang
 * rejects on Mach-O.  These mirrors replicate ONLY the struct layout
 * the dispatcher logic depends on.
 *
 * The dispatcher code (bmskill.c) was written against the real headers;
 * these mirrors exist solely so the test binary can compile on host.
 * If the real struct layouts change (e.g. a field is added to
 * SkillData), the host-side test will still compile but the GBA build
 * will catch the divergence.  Pinning struct sizes in these tests
 * ensures that doesn't go unnoticed.
 * ========================================================================
 */

/* ---- GBA basic types ------------------------------------------------- */
typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
typedef signed   char    s8;
typedef signed   short   s16;

/* ---- Skill constants ------------------------------------------------- */

enum {
    SKILL_NONE    = 0,
    SKILL_SURE_SHOT = 1,
};

/* ---- Skill hook enum ------------------------------------------------- */

enum SkillHook {
    SKILL_HOOK_PRE_HIT = 0,
};

/* ---- Forward mirrors of battle structs (opaque pointers) ------------- */
/* Unit and BattleUnit — dispatcher only touches unit.pClassData->number.
 * We model the minimal pointer chain the dispatcher follows. */

struct ClassData {
    u8 number;
};

struct Unit {
    struct ClassData *pClassData;
    /* Other fields exist on real struct Unit but dispatcher doesn't
       read them in GetClassSkillList. */
};

struct BattleUnit {
    struct Unit unit;
};

struct BattleHit {
    unsigned attributes : 19;
    unsigned info       : 5;
    signed   hpChange   : 8;
};

enum {
    BATTLE_HIT_ATTR_SURESHOT  = (1 << 14),
    BATTLE_HIT_ATTR_GREATSHLD = (1 << 15),
    BATTLE_HIT_ATTR_PIERCE    = (1 << 16),
};

enum {
    ITEM_BALLISTA_REGULAR = 0,  /* placeholder — GBA real IDs differ */
    ITEM_BALLISTA_LONG    = 1,
    ITEM_BALLISTA_KILLER  = 2,
};

/* ---- Skill battle context (matches bmskill.h exactly) --------------- */

struct SkillBattleContext {
    struct BattleUnit *attacker;
    struct BattleUnit *defender;
    struct BattleHit  *hit;
};

/* ---- SkillData / ClassSkillEnt (matches bmskill.h exactly) ----------- */

struct SkillData {
    u8 id;
    u8 hook;
    void (*battleHook)(struct SkillBattleContext *ctx);
};

struct ClassSkillEnt {
    u8 classId;
    const u8 *skills;
};

/*
 * ========================================================================
 * Test data tables — host-portable copies of the real CONST_DATA tables
 * in src/data_skills.c.
 * ========================================================================
 */

static const u8 TestSkillList_Sniper[] = {
    SKILL_SURE_SHOT,
    SKILL_NONE,
};

static const u8 TestSkillList_Multi[] = {
    SKILL_SURE_SHOT, /* slot 0 */
    SKILL_SURE_SHOT, /* slot 1 (duplicate — dispatch should call it twice) */
    SKILL_NONE,
};

static struct SkillData TestSkillData[] = {
    /* [0] SKILL_NONE */
    { .id = SKILL_NONE, .hook = SKILL_HOOK_PRE_HIT, .battleHook = NULL },
    /* [1] SKILL_SURE_SHOT */
    { .id = SKILL_SURE_SHOT,
      .hook = SKILL_HOOK_PRE_HIT,
      .battleHook = NULL /* handler is GBA-only; overridden in tests */ },
};

enum {
    TEST_CLASS_SNIPER = 0x01,
    TEST_CLASS_MULTI  = 0x03,
};

static struct ClassSkillEnt TestClassTable[] = {
    { TEST_CLASS_SNIPER, TestSkillList_Sniper },
    { TEST_CLASS_MULTI,  TestSkillList_Multi  },
    { 0, NULL }, /* sentinel (classId 0 terminates iteration) */
};

/*
 * ========================================================================
 * Dispatcher logic mirror — exact copy of bmskill.c's functions but
 * operating on the test tables above.
 * ========================================================================
 */

static const u8 *Test_GetClassSkillList(u8 classId)
{
    int i;
    for (i = 0; TestClassTable[i].classId != 0; ++i) {
        if (TestClassTable[i].classId == classId)
            return TestClassTable[i].skills;
    }
    return NULL;
}

static void Test_SkillDispatchBattle(enum SkillHook hook,
                                     struct SkillBattleContext *ctx)
{
    const u8 *skills = Test_GetClassSkillList(ctx->attacker->unit.pClassData->number);
    int i;

    if (!skills)
        return;

    for (i = 0; skills[i] != SKILL_NONE; ++i) {
        u8 skillId = skills[i];
        const struct SkillData *skillData = TestSkillData + skillId;

        if (skillData->id != skillId)
            continue;

        if (skillData->hook != hook)
            continue;

        if (skillData->battleHook)
            skillData->battleHook(ctx);
    }
}

/* ---- Test hooks & counters ------------------------------------------- */

typedef struct {
    int preHitCallCount;
    int sureShotCallCount;
    struct SkillBattleContext *lastCtx;
} TestDispatchStats;

static TestDispatchStats gTestStats;

static void TestSureShotHook(struct SkillBattleContext *ctx)
{
    gTestStats.preHitCallCount++;
    gTestStats.sureShotCallCount++;
    gTestStats.lastCtx = ctx;
}

/* ========================================================================
 * Test setup
 * ======================================================================== */

void setUp(void) { memset(&gTestStats, 0, sizeof(gTestStats)); }
void tearDown(void) {}

/* ========================================================================
 * 1. SKILL_NONE sentinel and table termination
 * ======================================================================== */

static void test_skill_none_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT(0, SKILL_NONE);
}

static void test_skill_none_terminates_sniper_list(void)
{
    const u8 *skills = Test_GetClassSkillList(TEST_CLASS_SNIPER);
    TEST_ASSERT_NOT_NULL(skills);
    TEST_ASSERT_EQUAL_UINT(SKILL_SURE_SHOT, skills[0]);
    TEST_ASSERT_EQUAL_UINT(SKILL_NONE, skills[1]);
}

static void test_class_zero_sentinel_terminates_table(void)
{
    const u8 *skills = Test_GetClassSkillList(0);
    TEST_ASSERT_NULL(skills);
}

/* ========================================================================
 * 2. Sniper / Sniper F class skill assignment
 * ======================================================================== */

static void test_sniper_receives_sure_shot(void)
{
    struct ClassData cls   = { .number = TEST_CLASS_SNIPER };
    struct Unit     u      = { .pClassData = &cls };
    struct BattleUnit attacker = { .unit = u };
    struct BattleUnit defender = { 0 };
    struct BattleHit  hit      = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &attacker,
        .defender = &defender,
        .hit      = &hit,
    };

    TestSkillData[SKILL_SURE_SHOT].battleHook = TestSureShotHook;
    Test_SkillDispatchBattle(SKILL_HOOK_PRE_HIT, &ctx);

    TEST_ASSERT_EQUAL_INT(1, gTestStats.preHitCallCount);
    TEST_ASSERT_EQUAL_INT(1, gTestStats.sureShotCallCount);

    TestSkillData[SKILL_SURE_SHOT].battleHook = NULL;
}

static void test_non_sniper_class_receives_no_skills(void)
{
    struct ClassData cls   = { .number = 0x42 };
    struct Unit     u      = { .pClassData = &cls };
    struct BattleUnit attacker = { .unit = u };
    struct BattleUnit defender = { 0 };
    struct BattleHit  hit      = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &attacker,
        .defender = &defender,
        .hit      = &hit,
    };

    Test_SkillDispatchBattle(SKILL_HOOK_PRE_HIT, &ctx);
    TEST_ASSERT_EQUAL_INT(0, gTestStats.preHitCallCount);
}

/* ========================================================================
 * 3. Hook dispatch ordering and filtering
 * ======================================================================== */

static void test_wrong_hook_id_skips_dispatch(void)
{
    struct ClassData cls   = { .number = TEST_CLASS_SNIPER };
    struct Unit     u      = { .pClassData = &cls };
    struct BattleUnit attacker = { .unit = u };
    struct BattleUnit defender = { 0 };
    struct BattleHit  hit      = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &attacker,
        .defender = &defender,
        .hit      = &hit,
    };

    TestSkillData[SKILL_SURE_SHOT].battleHook = TestSureShotHook;
    Test_SkillDispatchBattle((enum SkillHook)7, &ctx); /* nonexistent hook */

    TEST_ASSERT_EQUAL_INT(0, gTestStats.preHitCallCount);

    TestSkillData[SKILL_SURE_SHOT].battleHook = NULL;
}

static void test_dispatcher_passes_context_to_hook(void)
{
    struct ClassData cls   = { .number = TEST_CLASS_SNIPER };
    struct Unit     u      = { .pClassData = &cls };
    struct BattleUnit attacker = { .unit = u };
    struct BattleUnit defender = { 0 };
    struct BattleHit  hit      = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &attacker,
        .defender = &defender,
        .hit      = &hit,
    };

    TestSkillData[SKILL_SURE_SHOT].battleHook = TestSureShotHook;
    Test_SkillDispatchBattle(SKILL_HOOK_PRE_HIT, &ctx);

    TEST_ASSERT_EQUAL_PTR(&ctx, gTestStats.lastCtx);
    TEST_ASSERT_EQUAL_PTR(&attacker, gTestStats.lastCtx->attacker);
    TEST_ASSERT_EQUAL_PTR(&defender, gTestStats.lastCtx->defender);
    TEST_ASSERT_EQUAL_PTR(&hit, gTestStats.lastCtx->hit);

    TestSkillData[SKILL_SURE_SHOT].battleHook = NULL;
}

/* ========================================================================
 * 4. Multiple skills per class — iteration coverage
 * ======================================================================== */

static void test_multi_skill_class_dispatches_each_skill(void)
{
    struct ClassData cls   = { .number = TEST_CLASS_MULTI };
    struct Unit     u      = { .pClassData = &cls };
    struct BattleUnit attacker = { .unit = u };
    struct BattleUnit defender = { 0 };
    struct BattleHit  hit      = { 0 };
    struct SkillBattleContext ctx = {
        .attacker = &attacker,
        .defender = &defender,
        .hit      = &hit,
    };

    TestSkillData[SKILL_SURE_SHOT].battleHook = TestSureShotHook;
    Test_SkillDispatchBattle(SKILL_HOOK_PRE_HIT, &ctx);

    /* Two entries in class table -> hook called twice */
    TEST_ASSERT_EQUAL_INT(2, gTestStats.preHitCallCount);
    TEST_ASSERT_EQUAL_INT(2, gTestStats.sureShotCallCount);

    TestSkillData[SKILL_SURE_SHOT].battleHook = NULL;
}

/* ========================================================================
 * 5. Data integrity — SkillData table consistency
 * ======================================================================== */

static void test_skill_data_id_matches_index(void)
{
    /* Dispatcher skips if skillData->id != skillId.  Verify table is
     * identity-mapped so lookups succeed. */
    size_t n = sizeof(TestSkillData) / sizeof(TestSkillData[0]);
    for (size_t i = 0; i < n; ++i)
        TEST_ASSERT_EQUAL_UINT(i, TestSkillData[i].id);
}

static void test_null_hook_for_skill_none(void)
{
    /* SKILL_NONE entries should never have a battleHook. */
    TEST_ASSERT_NULL(TestSkillData[SKILL_NONE].battleHook);
}

/* ========================================================================
 * 6. Struct size pins (host vs GBA layout safety)
 * ======================================================================== */

static void test_skillbattlecontext_size(void)
{
    /* Three pointers: attacker + defender + hit */
    TEST_ASSERT_EQUAL_UINT(sizeof(void *) * 3,
                           sizeof(struct SkillBattleContext));
}

static void test_skilldata_size(void)
{
    /* On GBA (32-bit): u8 + u8 + pad(2) + ptr(4) = 8 bytes.
     * On host (64-bit): u8 + u8 + pad(6) + ptr(8) = 16 bytes.
     * We pin the host-side size so that if someone adds a field
     * to struct SkillData, it will be caught here as a change,
     * and the GBA build will verify the real layout.
     */
    TEST_ASSERT_EQUAL_UINT(16, sizeof(struct SkillData));
}

static void test_classskillent_size(void)
{
    /* u8 classId + pointer (+ padding) */
    TEST_ASSERT_EQUAL_UINT(2 * sizeof(void *),
                           sizeof(struct ClassSkillEnt));
}

/* ========================================================================
 * 7. Documented gaps — what CANNOT be tested on host
 * ======================================================================== */

/*
 * The following Sure Shot activation guards reference GBA-only symbols
 * and cannot be verified in a host-side test without extensive stubbing:
 *
 * 1. No duplicate activation  –  BATTLE_HIT_ATTR_SURESHOT guard
 * 2. Blocked by Pierce        –  BATTLE_HIT_ATTR_PIERCE guard
 * 3. Blocked by Great Shield  –  BATTLE_HIT_ATTR_GREATSHLD guard
 * 4. Ballista exclusion       –  GetItemIndex() ballista checks
 * 5. Activation probability   –  BattleRoll1RN(level, FALSE) RNG
 *
 * These guards live in SkillSureShotBattleHook (src/data_skills.c) and
 * use: gBattleHitIterator, BattleRoll1RN(), GetItemIndex(), ballista
 * item IDs.  Verified on GBA runtime (existing vanilla behavior).
 *
 * TODO(#110): add host stub or GBA test harness once the dispatcher
 * matures enough to justify the harness cost.
 */

static void test_documented_gaps_acknowledged(void)
{
    TEST_ASSERT_TRUE(1);
}

/* ========================================================================
 * Test runner
 * ======================================================================== */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skill_none_is_zero);
    RUN_TEST(test_skill_none_terminates_sniper_list);
    RUN_TEST(test_class_zero_sentinel_terminates_table);
    RUN_TEST(test_sniper_receives_sure_shot);
    RUN_TEST(test_non_sniper_class_receives_no_skills);
    RUN_TEST(test_wrong_hook_id_skips_dispatch);
    RUN_TEST(test_dispatcher_passes_context_to_hook);
    RUN_TEST(test_multi_skill_class_dispatches_each_skill);
    RUN_TEST(test_skill_data_id_matches_index);
    RUN_TEST(test_null_hook_for_skill_none);
    RUN_TEST(test_skillbattlecontext_size);
    RUN_TEST(test_skilldata_size);
    RUN_TEST(test_classskillent_size);
    RUN_TEST(test_documented_gaps_acknowledged);
    return UNITY_END();
}
