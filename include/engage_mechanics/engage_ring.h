#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_RING_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_RING_H

#include "gba/types.h"

// Provide a complete struct Unit so the ring-slot API surface below
// can take a `struct Unit *` parameter and consumers (tests, the
// umbrella re-export) can declare `struct Unit` locals without an
// extra #include. The host test build uses the canonical mirror because
// bmunit.h transitively includes ROM globals and section attributes that
// are not host-portable. The GBA build uses the authoritative definition.
#if defined(HOST_TEST)
#include "test_support/unit_mirror.h"
#else
#include "bmunit.h"
#endif

#define ENGAGE_RING_ITEM_COUNT 12
#define ENGAGE_RING_SLOT_NONE 0xFF

// Establish the ring-related invariants for a freshly cleared Unit.
// ClearUnit calls this after its zero-fill so ringSlot is the explicit
// no-ring sentinel instead of accidentally selecting items[0].
void InitUnitRingState(struct Unit *unit);

// Mark the ring stored at unit->items[ringItemId] as currently equipped.
// Caller must have already placed ringItemId in unit->items[]; the
// inventory-add path is coordinated separately. No-op if ringItemId is
// not in gRingItemDefs (e.g. ITEM_NONE, or a non-ring item id).
void EquipRing(struct Unit *unit, int ringItemId);

// Clear the equipped ring slot. Sets unit->ringSlot = ENGAGE_RING_SLOT_NONE.
void UnequipRing(struct Unit *unit);

// Return the item id of the currently equipped ring, or ITEM_NONE when
// no ring is equipped (unit->ringSlot == ENGAGE_RING_SLOT_NONE).
int GetEquippedRing(struct Unit *unit);

// Return the Emblem id associated with the currently equipped ring,
// or -1 when no ring is equipped or the equipped item id is not in
// gRingItemDefs.
int GetEquippedEmblemId(struct Unit *unit);

// Reverse lookup: return the ring item id that summons emblemId,
// or ITEM_NONE when no ring maps to emblemId.
int GetRingItemIdFromEmblemId(int emblemId);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_RING_H
