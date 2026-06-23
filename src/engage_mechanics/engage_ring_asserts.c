// Build-time size contract from issue #42 acceptance criteria.
// ringSlot and ringEngageState are both 1-byte fields repurposed from
// the existing _u3A/_u3B padding in struct Unit (no struct-size change).
//
// On macOS, the host test build cannot include bmunit.h (transitively
// pulls in variables.h, which uses Mach-O-incompatible section
// attributes). The canonical host-side mirror lives in
// tests/test_support/unit_mirror.h and carries offsetof static_asserts
// that ensure the mirror stays in sync with the GBA layout.
#if !defined(__APPLE__)
#include "bmunit.h"
#else
#include "test_support/unit_mirror.h"
#endif

_Static_assert(sizeof(((struct Unit *)0)->ringSlot) == 1,
               "ringSlot must be 1 byte");
_Static_assert(sizeof(((struct Unit *)0)->ringEngageState) == 1,
               "ringEngageState must be 1 byte");
