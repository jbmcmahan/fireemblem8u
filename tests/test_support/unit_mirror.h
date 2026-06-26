#ifndef GUARD_TEST_SUPPORT_UNIT_MIRROR_H
#define GUARD_TEST_SUPPORT_UNIT_MIRROR_H

#include <stddef.h>

/*
 * Canonical macOS host-side mirror of struct Unit (real definition:
 * include/bmunit.h). bmunit.h is not host-linkable on Apple because it
 * transitively includes variables.h, which uses Mach-O-incompatible
 * section attributes. This mirror exposes only the fields the engage
 * ring-slot API touches, at the same byte offsets as the GBA struct, so
 * host test code and the engage_mechanics TUs see one consistent layout.
 *
 * Consumers include this ONLY under #if defined(__APPLE__); on every
 * other platform they include the real bmunit.h.
 */
enum { UNIT_ITEM_COUNT = 5 };
struct Unit
{
    /* 00 */ unsigned char _pad_to_items[0x1E];
    /* 1E */ unsigned short items[UNIT_ITEM_COUNT];
    /* 28 */ unsigned char _pad_to_ringSlot[0x3A - 0x28];
    /* 3A */ unsigned char ringSlot;
    /* 3B */ unsigned char ringEngageState;
};

_Static_assert(offsetof(struct Unit, items) == 0x1E,
               "unit_mirror: items[] offset drifted from bmunit.h");
_Static_assert(offsetof(struct Unit, ringSlot) == 0x3A,
               "unit_mirror: ringSlot offset drifted from bmunit.h");
_Static_assert(offsetof(struct Unit, ringEngageState) == 0x3B,
               "unit_mirror: ringEngageState offset drifted from bmunit.h");

#endif /* GUARD_TEST_SUPPORT_UNIT_MIRROR_H */
