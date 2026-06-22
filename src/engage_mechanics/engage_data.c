#include "engage_mechanics/engage_data.h"

#include "engage_mechanics/engage_skills.h"
#include "engage_mechanics/engage_skill_registry.h"
#include "engage_mechanics/engage_bond_unlocks.h"
#include "engage_mechanics/engage_synced_bonus.h"

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

/**
 * gSkillDefs[64] — master skill registry.
 * Tail-appends after gEmblemDefs in engage_data.o(.data).
 * Layout: 64 * sizeof(struct SkillDef) = 64 * 2 = 128 bytes (0x80) -> ends
 *   0x090000D0. (SkillDef is u8+s8; alignment 1 on both host and GBA.)
 * index 0 = SKILL_EFFECT_NONE sentinel; indices 1..5 = generic kinds from
 *   #78 placeholder data (HP_PCT, BATTLE_ATK, BATTLE_HIT, BATTLE_AVO,
 *   BATTLE_CRIT); index 6 = BREAK, index 7 = DUAL_STRIKE placeholders;
 *   indices 8..15 = the new Marth-specific skill names from the #86 wiki
 *   table (PERCEPTIVE, PERCEPTIVE_PLUS, BREAK_DEFENSES, UNYIELDING,
 *   UNYIELDING_PLUS, UNYIELDING_PLUS_PLUS, SWORD_AGILITY, AVOID_BONUS).
 * Indices 16..63 zero-init per C99.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct SkillDef gSkillDefs[SKILL_DEF_COUNT] =
{
    [0] = { .kind = SKILL_EFFECT_NONE,             .value =  0 },
    [1] = { .kind = SKILL_EFFECT_HP_PCT,           .value =  5 },
    [2] = { .kind = SKILL_EFFECT_BATTLE_ATK,       .value =  2 },
    [3] = { .kind = SKILL_EFFECT_BATTLE_HIT,       .value =  5 },
    [4] = { .kind = SKILL_EFFECT_BATTLE_AVO,       .value = 10 },
    [5] = { .kind = SKILL_EFFECT_BATTLE_CRIT,      .value =  5 },
    [6] = { .kind = SKILL_EFFECT_NONE,             .value =  0 }, // was BREAK placeholder
    [7] = { .kind = SKILL_EFFECT_DUAL_STRIKE,      .value =  0 }, // Marth's engage skill from #78
    [8] = { .kind = SKILL_EFFECT_PERCEPTIVE,       .value =  0 },
    [9] = { .kind = SKILL_EFFECT_PERCEPTIVE_PLUS,  .value =  0 },
    [10] = { .kind = SKILL_EFFECT_BREAK_DEFENSES,  .value =  0 },
    [11] = { .kind = SKILL_EFFECT_UNYIELDING,      .value =  0 },
    [12] = { .kind = SKILL_EFFECT_UNYIELDING_PLUS, .value =  0 },
    [13] = { .kind = SKILL_EFFECT_UNYIELDING_PLUS_PLUS, .value = 0 },
    [14] = { .kind = SKILL_EFFECT_SWORD_AGILITY,   .value =  0 },
    [15] = { .kind = SKILL_EFFECT_AVOID_BONUS,     .value =  0 },
    /* indices 16..63 zero-init per C99 */
};

/**
 * gSyncSkillUnlocks[] tail-appends after gSkillDefs (starts 0x090000D0).
 * Size: gSyncSkillUnlockCount * sizeof(struct SkillUnlock) bytes.
 *   Each row is u8 + u8 + u8 = 3 bytes. 16 rows (Marth 5, Celica 5, Ike 5,
 *   plus Marth's 6th slot from #78) = 48 bytes (0x30) -> ends 0x09000100.
 * Translated from the old gSyncSkillDefs (PR #78). One unlock per populated
 * skill slot. .emblemId + .bondLevel + .skillId identify the unlock event;
 * .skillId is an index into gSkillDefs[]. Marth/Celica/Ike hand-curated;
 * the other 9 emblems get zero-init (no unlock rows).
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct SkillUnlock gSyncSkillUnlocks[] =
{
    /* Marth — 5 sync skills (tiers 1/3/5/9/15), migrated from #78 */
    { .emblemId =  0, .bondLevel =  1, .skillId = 1 }, // HP_PCT +5
    { .emblemId =  0, .bondLevel =  3, .skillId = 6 }, // BREAK (placeholder)
    { .emblemId =  0, .bondLevel =  5, .skillId = 2 }, // BATTLE_ATK +2
    { .emblemId =  0, .bondLevel =  9, .skillId = 3 }, // BATTLE_HIT +5
    { .emblemId =  0, .bondLevel = 15, .skillId = 4 }, // BATTLE_AVO +10
    /* Celica — 5 sync skills */
    { .emblemId =  1, .bondLevel =  1, .skillId = 1 }, // HP_PCT +5
    { .emblemId =  1, .bondLevel =  3, .skillId = 3 }, // BATTLE_HIT +5
    { .emblemId =  1, .bondLevel =  5, .skillId = 5 }, // BATTLE_CRIT +10
    { .emblemId =  1, .bondLevel =  9, .skillId = 4 }, // BATTLE_AVO +10
    { .emblemId =  1, .bondLevel = 15, .skillId = 1 }, // HP_PCT +10
    /* Ike — 5 sync skills, Atk-heavy */
    { .emblemId =  7, .bondLevel =  1, .skillId = 1 }, // HP_PCT +5
    { .emblemId =  7, .bondLevel =  3, .skillId = 2 }, // BATTLE_ATK +3
    { .emblemId =  7, .bondLevel =  5, .skillId = 5 }, // BATTLE_CRIT +5
    { .emblemId =  7, .bondLevel =  9, .skillId = 3 }, // BATTLE_HIT +10
    { .emblemId =  7, .bondLevel = 15, .skillId = 2 }, // BATTLE_ATK +5
};
const u8 gSyncSkillUnlockCount = sizeof(gSyncSkillUnlocks) / sizeof(gSyncSkillUnlocks[0]);

/**
 * gInheritSkillUnlocks[] — initially empty.
 * Tail-appends after gSyncSkillUnlocks (starts 0x09000100).
 * Size: gInheritSkillUnlockCount * sizeof(struct SkillUnlock) = 0 bytes.
 *   -> ends 0x09000100 (no data emitted; count extern still defined).
 * Marth's 6 inheritable skills (Perceptive, etc.) land in #85-C.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct SkillUnlock gInheritSkillUnlocks[1] = { 0 }; /* placeholder so the array has a definition */
const u8 gInheritSkillUnlockCount = 0;

/**
 * gEngageSkillUnlocks[] tail-appends after gInheritSkillUnlocks.
 * Size: 12 * sizeof(struct SkillUnlock) = 12 * 3 = 36 bytes (0x24)
 *   -> ends 0x09000124.
 * One engage skill per Emblem in canonical order. Marth (id 0) gets DUAL_STRIKE
 * (skillId = 7) translated from the old gEngageSkillDefs (PR #78). The other 11
 * entries point at skillId = 0 (SKILL_EFFECT_NONE sentinel) until later data issues.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct SkillUnlock gEngageSkillUnlocks[EMBLEM_DEF_COUNT] =
{
    [ 0] = { .emblemId =  0, .bondLevel = 0, .skillId = 7 }, // Marth — DUAL_STRIKE
    [ 1] = { .emblemId =  1, .bondLevel = 0, .skillId = 0 }, // Celica
    [ 2] = { .emblemId =  2, .bondLevel = 0, .skillId = 0 }, // Sigurd
    [ 3] = { .emblemId =  3, .bondLevel = 0, .skillId = 0 }, // Leif
    [ 4] = { .emblemId =  4, .bondLevel = 0, .skillId = 0 }, // Roy
    [ 5] = { .emblemId =  5, .bondLevel = 0, .skillId = 0 }, // Lyn
    [ 6] = { .emblemId =  6, .bondLevel = 0, .skillId = 0 }, // Eirika
    [ 7] = { .emblemId =  7, .bondLevel = 0, .skillId = 0 }, // Ike
    [ 8] = { .emblemId =  8, .bondLevel = 0, .skillId = 0 }, // Micaiah
    [ 9] = { .emblemId =  9, .bondLevel = 0, .skillId = 0 }, // Lucina
    [10] = { .emblemId = 10, .bondLevel = 0, .skillId = 0 }, // Corrin
    [11] = { .emblemId = 11, .bondLevel = 0, .skillId = 0 }, // Byleth
};
const u8 gEngageSkillUnlockCount = sizeof(gEngageSkillUnlocks) / sizeof(gEngageSkillUnlocks[0]);

/**
 * gWeaponUnlocks[] — initially empty.
 * Tail-appends after gEngageSkillUnlocks (starts 0x09000124).
 * Size: gWeaponUnlockCount * sizeof(struct WeaponUnlock) = 0 bytes.
 *   -> ends 0x09000124 (no data emitted).
 * Marth's Rapier unlock lands in #85-C.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct WeaponUnlock gWeaponUnlocks[1] = { 0 };
const u8 gWeaponUnlockCount = 0;

/**
 * gClassChangeUnlocks[] — initially empty.
 * Tail-appends after gWeaponUnlocks.
 * Size: gClassChangeUnlockCount * sizeof(struct ClassChangeUnlock) = 0 bytes.
 *   -> ends 0x09000124 (no data emitted).
 * Class-change flag data lands in #85-C.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct ClassChangeUnlock gClassChangeUnlocks[1] = { 0 };
const u8 gClassChangeUnlockCount = 0;

/**
 * gSyncedBonuses[EMBLEM_DEF_COUNT][20] tail-appends after gClassChangeUnlocks.
 * Size: 12 * 20 * sizeof(struct BonusRow) = 240 * 4 = 960 bytes (0x3C0)
 *   -> ends 0x090004E4. (struct BonusRow is 28 bits used; compiler pads to 4.)
 * Cumulative stat bonus per (emblem, bondLevel). All rows zero-init per C99.
 * Marth's 20 populated rows land in #85-C.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct BonusRow gSyncedBonuses[EMBLEM_DEF_COUNT][20] = { 0 };