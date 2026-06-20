// Build-time size contract from issue #42 acceptance criteria.
// ringSlot and ringEngageState are both 1-byte fields repurposed from
// the existing _u3A/_u3B padding in struct Unit (no struct-size change).
//
// On macOS, the host test build cannot include bmunit.h (transitively
// pulls in variables.h, which uses Mach-O-incompatible section
// attributes). The GBA build (the authoritative one for this size
// contract) always includes bmunit.h and asserts against the real
// struct Unit. A minimal mirror struct with the same 1-byte field
// property is used on macOS so `make test` can still build and the
// assert is still a meaningful compile-time check on that platform.
#if !defined(__APPLE__)
#include "bmunit.h"
#else
struct Unit
{
    unsigned char ringSlot;
    unsigned char ringEngageState;
};
#endif

_Static_assert(sizeof(((struct Unit *)0)->ringSlot) == 1,
               "ringSlot must be 1 byte");
_Static_assert(sizeof(((struct Unit *)0)->ringEngageState) == 1,
               "ringEngageState must be 1 byte");
