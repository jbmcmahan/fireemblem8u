#include "unity.h"
#include "engage_mechanics/engage_api.h"

void setUp(void) {}
void tearDown(void) {}

// 0xFF is the contract "no ring" sentinel for unit->ringSlot.
#define RING_SLOT_NONE 0xFF

// Placeholder ring ids. Once #41 (2.1) lands, these will be replaced by the
// canonical ITEM_RING_* constants. Picked to be plausibly in the 12-entry
// gRingItemDefs table (Marth = index 0). If 2.1 picks different numeric
// values, this is a one-line edit.
static const int TEST_RING_ID = ITEM_RING_MARTH;

// items[2] = an arbitrary in-range slot.
static const int TEST_SLOT = 2;

// Equipping an item id that is not in gRingItemDefs must leave ringSlot alone.
static const int BAD_RING_ID = ITEM_NONE;

// Reset every field the ring-slot API touches. The struct Unit definition
// in bmunit.h reserves _u3A/_u3B as padding today; once #42 (2.2) renames
// them to ringSlot/ringEngageState this helper still compiles unchanged.
static void reset_unit(struct Unit *u)
{
    for (int i = 0; i < UNIT_ITEM_COUNT; ++i)
        u->items[i] = ITEM_NONE;
    u->ringSlot = RING_SLOT_NONE;
}

// 1. EquipRing sets unit->ringSlot to a non-0xFF value.
static void test_equip_ring_sets_slot(void)
{
    struct Unit unit;
    reset_unit(&unit);
    unit.items[TEST_SLOT] = TEST_RING_ID;

    EquipRing(&unit, TEST_RING_ID);

    TEST_ASSERT_NOT_EQUAL(RING_SLOT_NONE, unit.ringSlot);
}

// 2. GetEquippedRing returns the same item id passed to EquipRing.
static void test_get_equipped_ring_round_trip(void)
{
    struct Unit unit;
    reset_unit(&unit);
    unit.items[TEST_SLOT] = TEST_RING_ID;

    EquipRing(&unit, TEST_RING_ID);

    TEST_ASSERT_EQUAL_INT(TEST_RING_ID, GetEquippedRing(&unit));
}

// 3. GetEquippedEmblemId returns the Emblem id associated with the
//    equipped ring. Marth is emblem 0 in gEmblemDefs / gRingItemDefs order.
static void test_get_equipped_emblem_id(void)
{
    struct Unit unit;
    reset_unit(&unit);
    unit.items[TEST_SLOT] = TEST_RING_ID;

    EquipRing(&unit, TEST_RING_ID);

    TEST_ASSERT_EQUAL_INT(0, GetEquippedEmblemId(&unit));
}

// 4. UnequipRing sets ringSlot == 0xFF.
static void test_unequip_ring_clears_slot(void)
{
    struct Unit unit;
    reset_unit(&unit);
    unit.items[TEST_SLOT] = TEST_RING_ID;

    EquipRing(&unit, TEST_RING_ID);
    UnequipRing(&unit);

    TEST_ASSERT_EQUAL_UINT8(RING_SLOT_NONE, unit.ringSlot);
}

// 5. GetEquippedRing returns the no-ring sentinel when nothing is equipped.
static void test_get_equipped_ring_returns_none_when_empty(void)
{
    struct Unit unit;
    reset_unit(&unit);

    TEST_ASSERT_EQUAL_INT(ITEM_NONE, GetEquippedRing(&unit));
}

// 6. EquipRing with an item id not in gRingItemDefs is a no-op.
static void test_equip_ring_bad_id_noop(void)
{
    struct Unit unit;
    reset_unit(&unit);
    // Deliberately do not stage BAD_RING_ID in items[]: the API should bail
    // before it even inspects inventory.
    EquipRing(&unit, BAD_RING_ID);

    TEST_ASSERT_EQUAL_UINT8(RING_SLOT_NONE, unit.ringSlot);
    TEST_ASSERT_EQUAL_INT(ITEM_NONE, GetEquippedRing(&unit));
}

// 7. GetRingItemIdFromEmblemId round-trips through EquipRing + GetEquippedRing.
static void test_emblem_id_to_ring_item_round_trip(void)
{
    struct Unit unit;
    reset_unit(&unit);

    int ringId = GetRingItemIdFromEmblemId(0);
    TEST_ASSERT_NOT_EQUAL(ITEM_NONE, ringId);

    unit.items[TEST_SLOT] = ringId;
    EquipRing(&unit, ringId);

    TEST_ASSERT_EQUAL_INT(ringId, GetEquippedRing(&unit));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_equip_ring_sets_slot);
    RUN_TEST(test_get_equipped_ring_round_trip);
    RUN_TEST(test_get_equipped_emblem_id);
    RUN_TEST(test_unequip_ring_clears_slot);
    RUN_TEST(test_get_equipped_ring_returns_none_when_empty);
    RUN_TEST(test_equip_ring_bad_id_noop);
    RUN_TEST(test_emblem_id_to_ring_item_round_trip);
    return UNITY_END();
}
