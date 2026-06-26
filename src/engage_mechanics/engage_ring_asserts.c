// Build-time size contract from issue #42 acceptance criteria.
// ringSlot and ringEngageState are both 1-byte fields repurposed from
// the existing _u3A/_u3B padding in struct Unit (no struct-size change).
//
// The host test build cannot include bmunit.h because it pulls in ROM globals
// and section attributes. The canonical host-side mirror lives in
// tests/test_support/unit_mirror.h and carries offsetof static_asserts.
#if defined(HOST_TEST)
#include "test_support/unit_mirror.h"
#else
#include "bmunit.h"
#endif

typedef char ringSlot_MustBe1Byte[
    (sizeof(((struct Unit *)0)->ringSlot) == 1) ? 1 : -1
];
typedef char ringEngageState_MustBe1Byte[
    (sizeof(((struct Unit *)0)->ringEngageState) == 1) ? 1 : -1
];

#if !defined(HOST_TEST)
typedef char Unit_MustRemain0x4CBytes[(sizeof(struct Unit) == 0x4C) ? 1 : -1];
#endif
