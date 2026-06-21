#include "engage_mechanics/engage_api.h"

#include "constants/items.h"

void EquipRing(struct Unit *unit, int ringItemId)
{
    // Validate that ringItemId is a known ring by scanning gRingItemDefs
    // for an itemId match. The closed-form ITEM_RING_MARTH+i arithmetic
    // (issue #77) coupled this API to the table's storage order; the
    // scan is reorder-safe.
    int defIndex = -1;
    for (int i = 0; i < ENGAGE_RING_ITEM_COUNT; ++i)
    {
        if (gRingItemDefs[i].itemId == ringItemId)
        {
            defIndex = i;
            break;
        }
    }
    if (defIndex < 0)
    {
        // ringItemId is not a ring the library knows about: leave ringSlot
        // untouched so a bad caller cannot silently corrupt an equipped ring.
        return;
    }

    // Inventory prerequisite: the caller must have already placed
    // ringItemId inside unit->items[]. ringSlot is the inventory slot
    // index (0..UNIT_ITEM_COUNT-1), NOT the gRingItemDefs index, so
    // GetEquippedRing can resolve the item id by unit->items[ringSlot].
    for (int slot = 0; slot < UNIT_ITEM_COUNT; ++slot)
    {
        if (unit->items[slot] == ringItemId)
        {
            unit->ringSlot = (u8)slot;
            return;
        }
    }
    // The ring is known but not in inventory: no-op (per the spec,
    // inventory-add is coordinated by a separate caller path).
}

void UnequipRing(struct Unit *unit)
{
    unit->ringSlot = 0xFF;
}

int GetEquippedRing(struct Unit *unit)
{
    if (unit->ringSlot == 0xFF)
        return ITEM_NONE;
    return unit->items[unit->ringSlot];
}

int GetEquippedEmblemId(struct Unit *unit)
{
    int ringItemId = GetEquippedRing(unit);
    if (ringItemId == ITEM_NONE)
        return -1;

    for (int i = 0; i < ENGAGE_RING_ITEM_COUNT; ++i)
    {
        if (gRingItemDefs[i].itemId == ringItemId)
            return gRingItemDefs[i].emblemId;
    }
    return -1;
}

int GetRingItemIdFromEmblemId(int emblemId)
{
    if (emblemId < 0 || emblemId >= ENGAGE_RING_ITEM_COUNT)
        return ITEM_NONE;

    // Reorder-safe (issue #77): look up by table, not arithmetic.
    return gRingItemDefs[emblemId].itemId;
}
