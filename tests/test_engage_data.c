#include "unity.h"
#include "engage_mechanics/engage_api.h"
#include "constants/items.h"

void setUp(void) {}
void tearDown(void) {}

// 1. Pin the host size so accidental field additions break the build.
static void test_emdef_size_is_40_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(40, sizeof(struct EmblemDef));
}

// 2 + 3. Dimension assertions — pure compile-time on a sized extern.
static void test_table_has_twelve_entries(void)
{
    TEST_ASSERT_EQUAL_UINT(12, sizeof(gEmblemDefs) / sizeof(gEmblemDefs[0]));
}

static void test_table_has_twelve_emdefs_size(void)
{
    TEST_ASSERT_EQUAL_UINT(12 * sizeof(struct EmblemDef), sizeof(gEmblemDefs));
}

// 6 + 7. Structural array sizes — never dereference gEmblemDefs.
static void test_sync_skills_array_size_is_five(void)
{
    TEST_ASSERT_EQUAL_UINT(5, sizeof(((struct EmblemDef *)0)->syncSkills));
}

static void test_bond_growth_stats_array_size_is_seven(void)
{
    TEST_ASSERT_EQUAL_UINT(7, sizeof(((struct EmblemDef *)0)->bondGrowthStats));
}

// 4. Every entry's name pointer non-NULL.
static void test_each_emblems_name_non_null(void)
{
    for (unsigned i = 0; i < EMBLEM_DEF_COUNT; ++i)
        TEST_ASSERT_NOT_NULL(gEmblemDefs[i].name);
}

// 5. engageWeaponItemId matches the host-side EXPECTED_IDS.
static const unsigned char EXPECTED_IDS[EMBLEM_DEF_COUNT] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

static void test_each_emblems_engageweaponid_matches_index(void)
{
    for (unsigned i = 0; i < EMBLEM_DEF_COUNT; ++i)
        TEST_ASSERT_EQUAL_UINT(EXPECTED_IDS[i], gEmblemDefs[i].engageWeaponItemId);
}

// 8. Names match canonical order.
static const char * const CANONICAL_NAMES[EMBLEM_DEF_COUNT] =
{
    "Marth",   "Celica",  "Sigurd",  "Leif",
    "Roy",     "Lyn",     "Eirika",  "Ike",
    "Micaiah", "Lucina",  "Corrin",  "Byleth",
};

static void test_emblems_names_match_canonical(void)
{
    for (unsigned i = 0; i < EMBLEM_DEF_COUNT; ++i)
        TEST_ASSERT_EQUAL_STRING(CANONICAL_NAMES[i], gEmblemDefs[i].name);
}

// 9. Pin the host size so accidental field additions break the build.
//    Issue #77: added `itemId` field; sizeof grew from 1 to 2 bytes.
static void test_rdef_size_is_2_bytes(void)
{
    TEST_ASSERT_EQUAL_UINT(2, sizeof(struct RingItemDef));
}

// 10. Dimension assertion — pure compile-time on a sized extern.
static void test_ring_table_has_twelve_entries(void)
{
    TEST_ASSERT_EQUAL_UINT(12, sizeof(gRingItemDefs) / sizeof(gRingItemDefs[0]));
}

// 11. Acceptance-criterion size check from issue #41.
static void test_ring_table_has_twelve_rdefs_size(void)
{
    TEST_ASSERT_EQUAL_UINT(12 * sizeof(struct RingItemDef), sizeof(gRingItemDefs));
}

// 12. Per-entry emblemId matches its index (canonical Emblem order).
static void test_each_rings_emblemid_matches_index(void)
{
    for (unsigned i = 0; i < 12; ++i)
        TEST_ASSERT_EQUAL_UINT(i, gRingItemDefs[i].emblemId);
}

// 13. The 12 new ring item IDs are contiguous and start at 0xCE
//     (after ITEM_UNK_CD = 0xCD). No collision with existing items.
static void test_ring_item_ids_are_contiguous(void)
{
    TEST_ASSERT_EQUAL_INT(0xCE, ITEM_RING_MARTH);
    TEST_ASSERT_EQUAL_INT(0xCF, ITEM_RING_CELICA);
    TEST_ASSERT_EQUAL_INT(0xD0, ITEM_RING_SIGURD);
    TEST_ASSERT_EQUAL_INT(0xD1, ITEM_RING_LEIF);
    TEST_ASSERT_EQUAL_INT(0xD2, ITEM_RING_ROY);
    TEST_ASSERT_EQUAL_INT(0xD3, ITEM_RING_LYN);
    TEST_ASSERT_EQUAL_INT(0xD4, ITEM_RING_EIRIKA);
    TEST_ASSERT_EQUAL_INT(0xD5, ITEM_RING_IKE);
    TEST_ASSERT_EQUAL_INT(0xD6, ITEM_RING_MICAIAH);
    TEST_ASSERT_EQUAL_INT(0xD7, ITEM_RING_LUCINA);
    TEST_ASSERT_EQUAL_INT(0xD8, ITEM_RING_CORRIN);
    TEST_ASSERT_EQUAL_INT(0xD9, ITEM_RING_BYLETH);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_emdef_size_is_40_bytes);
    RUN_TEST(test_table_has_twelve_entries);
    RUN_TEST(test_table_has_twelve_emdefs_size);
    RUN_TEST(test_sync_skills_array_size_is_five);
    RUN_TEST(test_bond_growth_stats_array_size_is_seven);
    RUN_TEST(test_each_emblems_name_non_null);
    RUN_TEST(test_each_emblems_engageweaponid_matches_index);
    RUN_TEST(test_emblems_names_match_canonical);
    RUN_TEST(test_rdef_size_is_2_bytes);
    RUN_TEST(test_ring_table_has_twelve_entries);
    RUN_TEST(test_ring_table_has_twelve_rdefs_size);
    RUN_TEST(test_each_rings_emblemid_matches_index);
    RUN_TEST(test_ring_item_ids_are_contiguous);
    return UNITY_END();
}
