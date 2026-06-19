#include "engage_mechanics/engage_data.h"

// CONST_DATA places gEmblemDefs in the ROM's .data section on the GBA build,
// matching the ldscript free-space region for engage_data.o. The host test
// build links engage_data.c into a Mach-O binary on macOS, where bare ".data"
// section attributes are rejected (clang errors with "mach-o section
// specifier requires a segment and section separated by a comma"). Override
// the attribute on __APPLE__ so the test binary compiles; on Linux/other
// ELFs the GCC section attribute is accepted.
#if !defined(CONST_DATA)
#  if defined(__APPLE__)
#    define CONST_DATA
#  elif defined(__GNUC__)
#    define CONST_DATA __attribute__((section(".data")))
#  else
#    define CONST_DATA
#  endif
#endif

// gEmblemDefs[12] — canonical Emblems in Engage order.
// Names match the test contract in tests/test_engage_data.c.
// engageWeaponItemId uses 0..11 placeholders; real items.h IDs land
// in a future data-expansion issue.
// All other struct EmblemDef fields zero-init per C.
CONST_DATA struct EmblemDef gEmblemDefs[EMBLEM_DEF_COUNT] =
{
    [ 0] = { .name = "Marth",   .engageWeaponItemId = 0  },
    [ 1] = { .name = "Celica",  .engageWeaponItemId = 1  },
    [ 2] = { .name = "Sigurd",  .engageWeaponItemId = 2  },
    [ 3] = { .name = "Leif",    .engageWeaponItemId = 3  },
    [ 4] = { .name = "Roy",     .engageWeaponItemId = 4  },
    [ 5] = { .name = "Lyn",     .engageWeaponItemId = 5  },
    [ 6] = { .name = "Eirika",  .engageWeaponItemId = 6  },
    [ 7] = { .name = "Ike",     .engageWeaponItemId = 7  },
    [ 8] = { .name = "Micaiah", .engageWeaponItemId = 8  },
    [ 9] = { .name = "Lucina",  .engageWeaponItemId = 9  },
    [10] = { .name = "Corrin",  .engageWeaponItemId = 10 },
    [11] = { .name = "Byleth",  .engageWeaponItemId = 11 },
};
