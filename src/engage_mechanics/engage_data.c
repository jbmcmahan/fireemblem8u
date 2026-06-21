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
// engageWeaponItemId uses 0..11 placeholders; real items.h IDs land
// in a future data-expansion issue.
// All other struct EmblemDef fields zero-init per C.
//
/**
 * gEmblemDefs lives at ROM address 0x08FFFF00 (see ldscript.txt line ~1215).
 * Size: 12 * sizeof(struct EmblemDef) = 12 * 28 = 336 bytes (0x150).
 *   (sizeof is 28 on GBA — pointer is 4 bytes; on host it's 40.)
 * Placement: tail-appended after data/data_FFF000.o(.data) at 0x08FFFF00.
 *   The first 256 bytes of the table overwrite a previously-zero ROM tail;
 *   the trailing 80 bytes extend the ROM past 0x09000000 (now 0x09000050).
 *   SHA1 changes accordingly — see checksum.sha1.
 * String literals (.rodata) live at 0x0858791C, 82 bytes (0x52).
 * If you grow this table, re-audit the gap and update this comment.
 */
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

// gRingItemDefs[12] — one ring item per Emblem, in gEmblemDefs order.
// Each entry's emblemId matches its index in gEmblemDefs[]. The itemId
// stores the actual ITEM_RING_* id so engage_ring.c can resolve ring
// lookups by table scan instead of closed-form ITEM_RING_MARTH+i
// arithmetic (issue #77). The order is owned by this TU and may be
// reordered without affecting the API.
//
// Placement: immediately after gEmblemDefs in engage_data.o(.data).
//   gEmblemDefs lives at 0x08FFFF00, size 336 bytes (12 * 28 on GBA).
//   gRingItemDefs starts at 0x09000050, size 24 bytes (12 * 2 on GBA).
//   ROM file now ends at 0x09000068 (was 0x0900005C after #41).
//   SHA1 changes accordingly — see checksum.sha1.
// Verification: deferred to issue #39 (dead-bytes audit, still open).
CONST_DATA struct RingItemDef gRingItemDefs[12] =
{
    [ 0] = { .itemId = ITEM_RING_MARTH,   .emblemId = 0  },  // Marth
    [ 1] = { .itemId = ITEM_RING_CELICA,  .emblemId = 1  },  // Celica
    [ 2] = { .itemId = ITEM_RING_SIGURD,  .emblemId = 2  },  // Sigurd
    [ 3] = { .itemId = ITEM_RING_LEIF,    .emblemId = 3  },  // Leif
    [ 4] = { .itemId = ITEM_RING_ROY,     .emblemId = 4  },  // Roy
    [ 5] = { .itemId = ITEM_RING_LYN,     .emblemId = 5  },  // Lyn
    [ 6] = { .itemId = ITEM_RING_EIRIKA,  .emblemId = 6  },  // Eirika
    [ 7] = { .itemId = ITEM_RING_IKE,     .emblemId = 7  },  // Ike
    [ 8] = { .itemId = ITEM_RING_MICAIAH, .emblemId = 8  },  // Micaiah
    [ 9] = { .itemId = ITEM_RING_LUCINA,  .emblemId = 9  },  // Lucina
    [10] = { .itemId = ITEM_RING_CORRIN,  .emblemId = 10 },  // Corrin
    [11] = { .itemId = ITEM_RING_BYLETH,  .emblemId = 11 },  // Byleth
};

// Build-time size contract from issue #41 acceptance criteria.
_Static_assert(sizeof(gRingItemDefs) == 12 * sizeof(struct RingItemDef),
               "gRingItemDefs must have 12 RingItemDef entries");
