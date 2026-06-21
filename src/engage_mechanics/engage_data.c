#include "engage_mechanics/engage_data.h"

#include "engage_mechanics/engage_skills.h"

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
 * gSyncSkillDefs tail-appends after gEmblemDefs in engage_data.o(.data).
 * gEmblemDefs ends at 0x09000050 (0x08FFFF00 + 0x150); this table starts there.
 * Layout: per-emblem { u8 count; struct SkillDef skills[MAX]; } block.
 *   sizeof(struct EmblemSyncSkills) = 1 + MAX*8 = 1 + 6*8 = 49 bytes on BOTH
 *   host and GBA (every field is u8/s8 — alignment is 1).
 *   Total = 12 * 49 = 588 bytes (0x24C) -> ends 0x0900029C.
 * Per-emblem slot count varies in real data (Marth = 6, others = 5); .count
 * tracks the live size and unused slots in .skills[] zero-init per C99.
 * Marth/Celica/Ike hand-curated; other 9 Emblems get HP+5 placeholder at tier 1.
 * Extends the ROM tail further past 0x09000000 (audit-by-inspection; this fork
 * does not gate on checksum.sha1 — see AGENTS.md/CONTRIBUTING.md).
 * If you grow this table, re-audit the gap and update this comment.
 */
CONST_DATA struct EmblemSyncSkills gSyncSkillDefs[EMBLEM_DEF_COUNT] =
{
    /* Marth — 6 sync skills (tiers 1/3/5/9/15 + 6th slot TBD) */
    [ 0] = {
        .count = 6,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT,     .value =  5, .emblemId = 0, .tier =  1 },
            [1] = { .kind = SKILL_EFFECT_BREAK,      .value =  0, .emblemId = 0, .tier =  3 },
            [2] = { .kind = SKILL_EFFECT_BATTLE_ATK, .value =  2, .emblemId = 0, .tier =  5 },
            [3] = { .kind = SKILL_EFFECT_BATTLE_HIT, .value =  5, .emblemId = 0, .tier =  9 },
            [4] = { .kind = SKILL_EFFECT_BATTLE_AVO, .value = 10, .emblemId = 0, .tier = 15 },
            /* [5] = Marth's 6th sync skill — TBD (zero-init placeholder) */
        },
    },
    /* Celica — 5 sync skills */
    [ 1] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT,     .value =  5, .emblemId = 1, .tier =  1 },
            [1] = { .kind = SKILL_EFFECT_BATTLE_HIT, .value =  5, .emblemId = 1, .tier =  3 },
            [2] = { .kind = SKILL_EFFECT_BATTLE_CRIT,.value = 10, .emblemId = 1, .tier =  5 },
            [3] = { .kind = SKILL_EFFECT_BATTLE_AVO, .value = 10, .emblemId = 1, .tier =  9 },
            [4] = { .kind = SKILL_EFFECT_HP_PCT,     .value = 10, .emblemId = 1, .tier = 15 },
        },
    },
    /* Sigurd — 5 sync skills, placeholder HP+5 @ tier 1 (remaining TBD) */
    [ 2] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 2, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Leif — 5 sync skills, placeholder */
    [ 3] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 3, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Roy — 5 sync skills, placeholder */
    [ 4] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 4, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Lyn — 5 sync skills, placeholder */
    [ 5] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 5, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Eirika — 5 sync skills, placeholder */
    [ 6] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 6, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Ike — 5 sync skills, Atk-heavy */
    [ 7] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT,     .value =  5, .emblemId = 7, .tier =  1 },
            [1] = { .kind = SKILL_EFFECT_BATTLE_ATK, .value =  3, .emblemId = 7, .tier =  3 },
            [2] = { .kind = SKILL_EFFECT_BATTLE_CRIT,.value =  5, .emblemId = 7, .tier =  5 },
            [3] = { .kind = SKILL_EFFECT_BATTLE_HIT, .value = 10, .emblemId = 7, .tier =  9 },
            [4] = { .kind = SKILL_EFFECT_BATTLE_ATK, .value =  5, .emblemId = 7, .tier = 15 },
        },
    },
    /* Micaiah — 5 sync skills, placeholder */
    [ 8] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 8, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Lucina — 5 sync skills, placeholder */
    [ 9] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 9, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Corrin — 5 sync skills, placeholder */
    [10] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 10, .tier = 1 },
            /* [1..4] TBD */
        },
    },
    /* Byleth — 5 sync skills, placeholder */
    [11] = {
        .count = 5,
        .skills = {
            [0] = { .kind = SKILL_EFFECT_HP_PCT, .value = 5, .emblemId = 11, .tier = 1 },
            /* [1..4] TBD */
        },
    },
};

/**
 * gEngageSkillDefs tail-appends after gSyncSkillDefs (starts 0x0900029C).
 * Size: EMBLEM_DEF_COUNT * SKILL_DEF_COUNT_ENGAGE * sizeof(struct SkillDef)
 *   = 12 * 1 * 8 = 96 bytes (0x60) -> ends 0x090002FC.
 * One engage skill per Emblem in canonical order. Only Marth is populated for now
 * (DUAL_STRIKE); the other 11 are SKILL_EFFECT_NONE until later data issues.
 */
CONST_DATA struct SkillDef gEngageSkillDefs[EMBLEM_DEF_COUNT * SKILL_DEF_COUNT_ENGAGE] =
{
    [ 0] = { .kind = SKILL_EFFECT_DUAL_STRIKE, .value = 0, .emblemId =  0, .tier = 0 }, // Marth
    [ 1] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  1, .tier = 0 }, // Celica
    [ 2] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  2, .tier = 0 }, // Sigurd
    [ 3] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  3, .tier = 0 }, // Leif
    [ 4] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  4, .tier = 0 }, // Roy
    [ 5] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  5, .tier = 0 }, // Lyn
    [ 6] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  6, .tier = 0 }, // Eirika
    [ 7] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  7, .tier = 0 }, // Ike
    [ 8] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  8, .tier = 0 }, // Micaiah
    [ 9] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId =  9, .tier = 0 }, // Lucina
    [10] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId = 10, .tier = 0 }, // Corrin
    [11] = { .kind = SKILL_EFFECT_NONE,        .value = 0, .emblemId = 11, .tier = 0 }, // Byleth
};
