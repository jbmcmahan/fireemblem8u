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

// 2. Sync skill unlock table has 16 entries (#85-C adds Marth's 6th unlock).
//    Marth 6 + Celica 5 + Ike 5 = 16.
static void test_sync_skill_unlock_count_is_16(void)
{
    TEST_ASSERT_EQUAL_UINT(16, gSyncSkillUnlockCount);
}

// 3. Inherit/Weapon/ClassChange unlock tables populated with Marth's data (#85-C).
//    Inherit: Marth's 6 inheritable skills (Lv 1/2/4/7/9/12).
static void test_inherit_skill_unlock_count_is_6(void)
{
    TEST_ASSERT_EQUAL_UINT(6, gInheritSkillUnlockCount);
}

//    Weapon: Marth's Rapier unlock (Lv 1, itemId 0 placeholder).
static void test_weapon_unlock_count_is_1(void)
{
    TEST_ASSERT_EQUAL_UINT(1, gWeaponUnlockCount);
}

//    Class change: Marth's class change (Lv 8).
static void test_class_change_unlock_count_is_1(void)
{
    TEST_ASSERT_EQUAL_UINT(1, gClassChangeUnlockCount);
}

// 4. Engage skill table has 12 entries (one per emblem).
static void test_engage_skill_unlock_count_is_12(void)
{
    TEST_ASSERT_EQUAL_UINT(EMBLEM_DEF_COUNT, gEngageSkillUnlockCount);
}

// 5. Marth's first sync skill unlock is Perceptive @ bond level 1 (#85-C wiki data).
static void test_marth_t1_sync_skill_is_perceptive(void)
{
    TEST_ASSERT_EQUAL_UINT(0,  gSyncSkillUnlocks[0].emblemId);
    TEST_ASSERT_EQUAL_UINT(1,  gSyncSkillUnlocks[0].bondLevel);
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_PERCEPTIVE,
                           gSkillDefs[gSyncSkillUnlocks[0].skillId].kind);
}

// 6. Marth's engage skill skillId indexes gSkillDefs[DIVINE_SPEED].
static void test_marth_engage_skill_is_divine_speed(void)
{
    TEST_ASSERT_EQUAL_UINT(SKILL_EFFECT_DIVINE_SPEED,
                           gSkillDefs[gEngageSkillUnlocks[0].skillId].kind);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_skill_unlock_size_is_3_bytes);
    RUN_TEST(test_weapon_unlock_size_is_3_bytes);
    RUN_TEST(test_class_change_unlock_size_is_2_bytes);
    RUN_TEST(test_sync_skill_unlock_count_is_16);
    RUN_TEST(test_inherit_skill_unlock_count_is_6);
    RUN_TEST(test_weapon_unlock_count_is_1);
    RUN_TEST(test_class_change_unlock_count_is_1);
    RUN_TEST(test_engage_skill_unlock_count_is_12);
    RUN_TEST(test_marth_t1_sync_skill_is_perceptive);
    RUN_TEST(test_marth_engage_skill_is_divine_speed);
    return UNITY_END();
}