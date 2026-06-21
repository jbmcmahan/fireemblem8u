#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_RING_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_RING_H

#include "gba/types.h"

// Provide a complete struct Unit so the ring-slot API surface below
// can take a `struct Unit *` parameter and consumers (tests, the
// umbrella re-export) can declare `struct Unit` locals without an
// extra #include. On the GBA build and the Linux host build, pull in
// the authoritative definition from bmunit.h. On macOS, bmunit.h is
// not host-linkable (it transitively includes variables.h, which uses
// Mach-O-incompatible section attributes); fall back to a minimal
// mirror that exposes only the fields the ring API touches. The same
// rationale appears in src/engage_mechanics/engage_ring_asserts.c.
#if !defined(__APPLE__)
#include "bmunit.h"
#else
enum { UNIT_ITEM_COUNT = 5 };
struct Unit
{
    /* 00 */ unsigned char _pad_to_items[0x1E];
    /* 1E */ unsigned short items[UNIT_ITEM_COUNT];
    /* 28 */ unsigned char _pad_to_ringSlot[0x3A - 0x28];
    /* 3A */ unsigned char ringSlot;        // 0..4 = index of equipped ring; 0xFF = none.
    /* 3B */ unsigned char ringEngageState; // 0 = idle; 1..N = turns remaining.
};
#endif

#define ENGAGE_RING_ITEM_COUNT 12

// Mark the ring stored at unit->items[ringItemId] as currently equipped.
// Caller must have already placed ringItemId in unit->items[]; the
// inventory-add path is coordinated separately. No-op if ringItemId is
// not in gRingItemDefs (e.g. ITEM_NONE, or a non-ring item id).
void EquipRing(struct Unit *unit, int ringItemId);

// Clear the equipped ring slot. Sets unit->ringSlot = 0xFF.
void UnequipRing(struct Unit *unit);

// Return the item id of the currently equipped ring, or ITEM_NONE when
// no ring is equipped (unit->ringSlot == 0xFF).
int GetEquippedRing(struct Unit *unit);

// Return the Emblem id associated with the currently equipped ring,
// or -1 when no ring is equipped or the equipped item id is not in
// gRingItemDefs.
int GetEquippedEmblemId(struct Unit *unit);

// Reverse lookup: return the ring item id that summons emblemId,
// or ITEM_NONE when no ring maps to emblemId.
int GetRingItemIdFromEmblemId(int emblemId);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_RING_H
