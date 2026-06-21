#include "engage_mechanics/engage_data.h"
#include "constants/items.h"

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
// engageWeaponItemId references the ITEM_EMBLEM_* enum in
// include/constants/items.h. All other struct EmblemDef fields zero-init per C.
//
/*
 * gEmblemDefs lives at ROM address 0x08FFFF00 (see ldscript.txt:1215).
 * next-symbol: none
 * Size: 12 * sizeof(struct EmblemDef) = 12 * 28 = 336 bytes (0x150).
 *   (sizeof is 28 on GBA — pointer is 4 bytes; on host it's 40.)
 * Boundaries:
 *   Previous-symbol: data_FFF000 (per ldscript.txt:1214).
 *   Next-symbol: none — last entry in the .data section.
 * Placement: tail-appended after data/data_FFF000.o(.data) at 0x08FFFF00.
 *   The first 256 bytes of the table overwrite a previously-0xFF ROM tail;
 *   the trailing 80 bytes extend the ROM past 0x09000000 (now 0x09000050).
 *   SHA1 changes accordingly — see checksum.sha1.
 * String literals (.rodata) live at 0x0858791C, 82 bytes (0x52).
 * If you grow this table (or add another .data object after it), re-audit
 * both Boundaries and Placement, and update this comment.
 *
 * Verification (issue #39): baserom.gba @ 0x08FFFF00..0x09000000 == all 0xFF.
 *   No symbols in fireemblem8.map cover this range — only gEmblemDefs.
 *   See ldscript.txt line 1212 (FILL(0xFF) from end of _banim_pal to end of
 *   .data section). See commit message for the report.
 */
CONST_DATA struct EmblemDef gEmblemDefs[EMBLEM_DEF_COUNT] =
{
    [ 0] = { .name = "Marth",   .engageWeaponItemId = ITEM_EMBLEM_MARTH   },
    [ 1] = { .name = "Celica",  .engageWeaponItemId = ITEM_EMBLEM_CELICA  },
    [ 2] = { .name = "Sigurd",  .engageWeaponItemId = ITEM_EMBLEM_SIGURD  },
    [ 3] = { .name = "Leif",    .engageWeaponItemId = ITEM_EMBLEM_LEIF    },
    [ 4] = { .name = "Roy",     .engageWeaponItemId = ITEM_EMBLEM_ROY     },
    [ 5] = { .name = "Lyn",     .engageWeaponItemId = ITEM_EMBLEM_LYN     },
    [ 6] = { .name = "Eirika",  .engageWeaponItemId = ITEM_EMBLEM_EIRIKA  },
    [ 7] = { .name = "Ike",     .engageWeaponItemId = ITEM_EMBLEM_IKE     },
    [ 8] = { .name = "Micaiah", .engageWeaponItemId = ITEM_EMBLEM_MICAIAH },
    [ 9] = { .name = "Lucina",  .engageWeaponItemId = ITEM_EMBLEM_LUCINA  },
    [10] = { .name = "Corrin",  .engageWeaponItemId = ITEM_EMBLEM_CORRIN  },
    [11] = { .name = "Byleth",  .engageWeaponItemId = ITEM_EMBLEM_BYLETH  },
};
