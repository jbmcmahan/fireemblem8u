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
    [14] = { .kind = SKILL_EFFECT_SWORD_AGILITY,   .value =  1 }, // tier 1
    [15] = { .kind = SKILL_EFFECT_AVOID_BONUS,     .value = 10 },
    [16] = { .kind = SKILL_EFFECT_SWORD_AGILITY,   .value =  2 }, // tier 2
    [17] = { .kind = SKILL_EFFECT_SWORD_AGILITY,   .value =  3 }, // tier 3
    [18] = { .kind = SKILL_EFFECT_AVOID_BONUS,     .value = 15 },
    [19] = { .kind = SKILL_EFFECT_AVOID_BONUS,     .value = 20 },
    /* indices 20..63 zero-init per C99 */
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
    /* Marth — 6 sync skills at bond levels 1, 3, 7, 12, 16, 18 (per #86 wiki table) */
    { .emblemId =  0, .bondLevel =  1, .skillId =  8 }, // Perceptive
    { .emblemId =  0, .bondLevel =  3, .skillId = 10 }, // Break Defenses
    { .emblemId =  0, .bondLevel =  7, .skillId = 11 }, // Unyielding
    { .emblemId =  0, .bondLevel = 12, .skillId = 12 }, // Unyielding+
    { .emblemId =  0, .bondLevel = 16, .skillId =  9 }, // Perceptive+
    { .emblemId =  0, .bondLevel = 18, .skillId = 13 }, // Unyielding++
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
 * gInheritSkillUnlocks[] tail-appends after gSyncSkillUnlocks (starts 0x09000118).
 * Size: 6 * sizeof(struct SkillUnlock) = 6 * 3 = 18 bytes (0x12)
 *   -> ends 0x0900012A.
 * Marth's 6 inheritable skills at bond levels 1, 2, 4, 7, 9, 12. The other
 * 11 emblems get no inheritable rows in #85-C; their unlock tables stay empty.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct SkillUnlock gInheritSkillUnlocks[] =
{
    /* Marth — 6 inheritable skills */
    { .emblemId =  0, .bondLevel =  1, .skillId =  8 }, // Perceptive (inheritable)
    { .emblemId =  0, .bondLevel =  2, .skillId = 15 }, // Avoid +10
    { .emblemId =  0, .bondLevel =  4, .skillId = 14 }, // Sword Agility 1
    { .emblemId =  0, .bondLevel =  7, .skillId = 11 }, // Unyielding (inheritable)
    { .emblemId =  0, .bondLevel =  9, .skillId = 16 }, // Sword Agility 2
    { .emblemId =  0, .bondLevel = 12, .skillId = 17 }, // Sword Agility 3
};
const u8 gInheritSkillUnlockCount = sizeof(gInheritSkillUnlocks) / sizeof(gInheritSkillUnlocks[0]);

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
 * gWeaponUnlocks[] tail-appends after gEngageSkillUnlocks (starts 0x09000148).
 * Size: 1 * sizeof(struct WeaponUnlock) = 1 * 3 = 3 bytes (0x3)
 *   -> ends 0x0900014B.
 * Marth's Rapier at bond Lv 1. itemId is 0 (placeholder) — ITEM_RAPIER
 * constant doesn't exist yet (tracked separately; not blocking #85).
 * The other 11 emblems have no weapon unlocks in #85-C.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct WeaponUnlock gWeaponUnlocks[] =
{
    /* Marth — Rapier (placeholder itemId 0) at bond Lv 1 */
    { .emblemId =  0, .bondLevel =  1, .itemId = 0 },
};
const u8 gWeaponUnlockCount = sizeof(gWeaponUnlocks) / sizeof(gWeaponUnlocks[0]);

/**
 * gClassChangeUnlocks[] tail-appends after gWeaponUnlocks (starts 0x0900014B).
 * Size: 1 * sizeof(struct ClassChangeUnlock) = 1 * 2 = 2 bytes (0x2)
 *   -> ends 0x0900014D.
 * Marth's class change unlocks at bond Lv 8. The other 11 emblems have
 * no class-change data in #85-C.
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct ClassChangeUnlock gClassChangeUnlocks[] =
{
    /* Marth — class change at bond Lv 8 */
    { .emblemId =  0, .bondLevel =  8 },
};
const u8 gClassChangeUnlockCount = sizeof(gClassChangeUnlocks) / sizeof(gClassChangeUnlocks[0]);

/**
 * gSyncedBonuses[EMBLEM_DEF_COUNT][20] tail-appends after gClassChangeUnlocks
 * (starts 0x0900014D). Size: 12 * 20 * sizeof(struct BonusRow) = 240 * 4 =
 * 960 bytes (0x3C0) -> ends 0x0900050D.
 * (struct BonusRow is 28 bits used; compiler pads to 4 bytes per row.)
 *
 * Cumulative stat bonus per (emblem, bondLevel). Engage bond bonuses are
 * monotonic (never decrease) so cumulative encoding is valid. mag nibble
 * stays 0 throughout (FE8's struct Unit has no .mag field yet).
 *
 * Marth's 20 rows (emblem 0) populated from the wiki bond-level table.
 * The other 11 emblems get zero-init rows (their data lands in follow-ups).
 *
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct BonusRow gSyncedBonuses[EMBLEM_DEF_COUNT][20] =
{
    /* Marth — cumulative bonuses at each bond level (Lv 1..Lv 20) */
    [0][0]  = { .hp = 0, .str = 1, .mag = 0, .skl = 0, .spd = 1, .def = 0, .res = 0 }, /* Lv 1  */
    [0][1]  = { .hp = 0, .str = 1, .mag = 0, .skl = 1, .spd = 1, .def = 0, .res = 0 }, /* Lv 2  */
    [0][2]  = { .hp = 0, .str = 2, .mag = 0, .skl = 1, .spd = 1, .def = 0, .res = 0 }, /* Lv 3  */
    [0][3]  = { .hp = 0, .str = 2, .mag = 0, .skl = 1, .spd = 1, .def = 1, .res = 0 }, /* Lv 4  */
    [0][4]  = { .hp = 1, .str = 2, .mag = 0, .skl = 1, .spd = 1, .def = 1, .res = 0 }, /* Lv 5  */
    [0][5]  = { .hp = 1, .str = 2, .mag = 0, .skl = 2, .spd = 1, .def = 1, .res = 0 }, /* Lv 6  */
    [0][6]  = { .hp = 1, .str = 3, .mag = 0, .skl = 2, .spd = 1, .def = 1, .res = 0 }, /* Lv 7  */
    [0][7]  = { .hp = 1, .str = 3, .mag = 0, .skl = 2, .spd = 2, .def = 1, .res = 0 }, /* Lv 8  */
    [0][8]  = { .hp = 1, .str = 3, .mag = 0, .skl = 2, .spd = 2, .def = 1, .res = 1 }, /* Lv 9  */
    [0][9]  = { .hp = 1, .str = 3, .mag = 0, .skl = 3, .spd = 2, .def = 1, .res = 1 }, /* Lv 10 */
    [0][10] = { .hp = 2, .str = 3, .mag = 0, .skl = 3, .spd = 2, .def = 1, .res = 1 }, /* Lv 11 */
    [0][11] = { .hp = 2, .str = 4, .mag = 0, .skl = 3, .spd = 2, .def = 1, .res = 1 }, /* Lv 12 */
    [0][12] = { .hp = 2, .str = 4, .mag = 0, .skl = 3, .spd = 3, .def = 1, .res = 1 }, /* Lv 13 */
    [0][13] = { .hp = 2, .str = 4, .mag = 0, .skl = 4, .spd = 3, .def = 1, .res = 1 }, /* Lv 14 */
    [0][14] = { .hp = 2, .str = 4, .mag = 0, .skl = 4, .spd = 3, .def = 2, .res = 1 }, /* Lv 15 */
    [0][15] = { .hp = 2, .str = 5, .mag = 0, .skl = 4, .spd = 3, .def = 2, .res = 1 }, /* Lv 16 */
    [0][16] = { .hp = 3, .str = 5, .mag = 0, .skl = 4, .spd = 3, .def = 2, .res = 1 }, /* Lv 17 */
    [0][17] = { .hp = 3, .str = 5, .mag = 0, .skl = 5, .spd = 3, .def = 2, .res = 1 }, /* Lv 18 */
    [0][18] = { .hp = 3, .str = 5, .mag = 0, .skl = 5, .spd = 4, .def = 2, .res = 1 }, /* Lv 19 */
    [0][19] = { .hp = 3, .str = 5, .mag = 0, .skl = 5, .spd = 4, .def = 2, .res = 2 }, /* Lv 20 */
    /* emblems 1..11 zero-init per C99 */
};