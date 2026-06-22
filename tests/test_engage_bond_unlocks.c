#include "unity.h"
#include "engage_mechanics/engage_api.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Pin host sizes so accidental field additions break the build.
static void test_skill_unlock_size_is_3_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(3, sizeof(struct SkillUnlock));
}

static void test_weapon_unlock_size_is_3_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(3, sizeof(struct WeaponUnlock));
}

static void test_class_change_unlock_size_is_2_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(2, sizeof(struct ClassChangeUnlock));
}

// 2. Sync skill unlock table has 15 entries from the #78 migration
//    (Marth 5 + Celica 5 + Ike 5).
static void test_sync_skill_unlock_count_is_15(void)
{
    TEST_ASSERT_EQUAL_UINT(15, gSyncSkillUnlockCount);
}

// 3. Inherit/Weapon/ClassChange unlock tables are empty (#85-A).
static void test_inherit_skill_unlock_count_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gInheritSkillUnlockCount);
}

static void test_weapon_unlock_count_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gWeaponUnlockCount);
}

static void test_class_change_unlock_count_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT(0, gClassChangeUnlockCount);
}

// 4. Engage skill table has 12 entries (one per emblem).
static void test_engage_skill_unlock_count_is_12(void)
{
    TEST_ASSERT_EQUAL_UINT(EMBLEM_DEF_COUNT, gEngageSkillUnlockCount);
}

// 5. Marth's first sync skill unlock is HP_PCT @ bond level 1
//    (mirrors the #78 placeholder data translated to the new model).
static void test_marth_t1_sync_skill_is_hp_pct(void)
{
    TEST_ASSERT_EQUAL_UINT(0,  gSyncSkillUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(1,  gSyncSkillUnlocks[0].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_HP_PCT,
                           gSkillDefs[gSyncSkillUnlocks[0].skillId].kind);
    TEST_ASSERT_EQUAL_INT(5,  gSkillDefs[gSyncSkillUnlocks[0].skillId].value);
}

// 6. Marth's engage skill skillId indexes gSkillDefs[DUAL_STRIKE].
static void test_marth_engage_skill_is_dual_strike(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_DUAL_STRIKE,
                           gSkillDefs[gEngageSkillUnlocks[0].skillId].kind);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skill_unlock_size_is_3_bytes);
    RUN_TEST(test_weapon_unlock_size_is_3_bytes);
    RUN_TEST(test_class_change_unlock_size_is_2_bytes);
    RUN_TEST(test_sync_skill_unlock_count_is_15);
    RUN_TEST(test_inherit_skill_unlock_count_is_zero);
    RUN_TEST(test_weapon_unlock_count_is_zero);
    RUN_TEST(test_class_change_unlock_count_is_zero);
    RUN_TEST(test_engage_skill_unlock_count_is_12);
    RUN_TEST(test_marth_t1_sync_skill_is_hp_pct);
    RUN_TEST(test_marth_engage_skill_is_dual_strike);
    return UNITY_END();
}