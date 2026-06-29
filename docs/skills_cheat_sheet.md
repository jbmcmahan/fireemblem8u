# Skills Implementation Cheat Sheet

Use this when adding a new combat, personal, class, ring, or map skill.

## First Question

Decide what kind of effect the skill is before writing code.

| Skill shape | Put it here | Why |
| --- | --- | --- |
| Per-hit random/proc effect, such as Sure Shot | `SkillDispatchForUnit` via `gSkillData[].battleHook` | It belongs to actual hit generation and can depend on per-hit RNG. |
| Deterministic combat stat change, such as Hit/Avo/Crit/Def | `SkillApplyBattleStatBonuses` | Forecast, AI, and real combat should see the same numbers. |
| Targeting/range rule, such as staff range | A narrow helper in `bmskill.c`, called by targeting code | Keeps map-target code from knowing individual skill ids. |
| Ring sync/engage stat effect | `src/engage_mechanics/engage_skills.c` | Ring data has its own tables and host-portable resolver tests. |
| Learned/passive ownership check | `UnitHasSkill` / `GetUnitSkills` | Ownership should stay table-driven. |

Avoid adding one-off checks deep inside unrelated systems unless the effect is truly local to that system.

## Forecast Rule

If a skill changes a number the player sees before choosing combat, apply it before effective battle stats are computed.

Good examples:

- `battleAttack`
- `battleDefense`
- `battleHitRate`
- `battleAvoidRate`
- `battleCritRate`
- staff target range

Bad place for deterministic changes:

- `BattleGenerateHit`
- `BattleGenerateHitAttributes`
- `gBattleStats.hitRate`
- `gBattleStats.damage`

Those paths are per-hit resolution paths. They are appropriate for proc skills and actual-hit effects, not for stable prediction-visible bonuses.

## Ownership Tables

Class and personal skills are source lists, not hardcoded branches.

- Add skill ids in `include/constants/skills.h`.
- Keep `SKILL_MAX` larger than the highest skill id.
- Add `gSkillData[SKILL_MAX]` entries in `src/data_skills.c`.
- Add class lists to `gClassSkillTable`.
- Add personal lists to `gCharSkillTable`.
- Terminate all skill lists with `SKILL_NONE`.

`gSkillData` should be indexed safely. If a skill id can come from data, guard `skillId >= SKILL_MAX` before indexing.

## Runtime State

Be very careful before adding fields to `struct Unit`.

`struct Unit` is multiplied across the blue/red/green/purple unit arrays in EWRAM. Even a few bytes can move fixed linker addresses and break the ROM link.

Preferred order:

1. Use existing padding only when the state is truly per-unit and stable.
2. Pack small state behind macros/accessors.
3. Use a side table only if padding is insufficient, and place it deliberately in `ldscript.txt`.
4. Do not move fixed EWRAM symbols just to make a new field fit.

Current Engage ring state is packed in `Unit._u3A/_u3B` behind:

- `UNIT_RING_EMBLEM_ID`
- `UNIT_RING_BOND_LEVEL`
- `UNIT_ENGAGE_SKILL_USED`
- `UNIT_SET_RING_EMBLEM_ID`
- `UNIT_SET_RING_BOND_LEVEL`
- `UNIT_SET_ENGAGE_SKILL_USED`
- `UNIT_CLEAR_ENGAGE_STATE`

Use those accessors. Do not read or write `_u3A/_u3B` directly outside the accessor definitions.

## Initialization And Save Data

If a new skill needs runtime state, answer these before committing:

- What is the blank/default value?
- Is zero a valid value?
- Should `ClearUnit` initialize it?
- Should game save persist it?
- Should suspend save persist it?
- Does arena/link arena need special handling?

For ring state, `0xFF` means no ring. Zero is a valid emblem id, so plain zero-fill is wrong.

Save structs may store clearer explicit fields even when runtime state is packed. Keep serialization through accessors so the representation stays centralized.

## Map Scans

Map-aware skills must bounds-check both lower and upper map coordinates before reading `gBmMapUnit[y][x]`.

Use a helper like:

```c
static int IsMapPositionInBounds(int x, int y)
{
    return x >= 0 && y >= 0 && x < gBmMapSize.x && y < gBmMapSize.y;
}
```

Also filter dead, undeployed, and hidden units when the skill cares about active allies.

## Battle Stat Skills

For deterministic battle skills:

- Apply unit-local bonuses to each side.
- Apply actor-to-target effects after both sides exist.
- Skip arena if the skill should not affect arena combat.
- Avoid mutating `gBattleStats` for stable bonuses.
- Clamp values that should not go below zero, such as avoid.

The current shared entry point is:

```c
void SkillApplyBattleStatBonuses(struct BattleUnit *actor,
                                 struct BattleUnit *target);
```

Call it from both real battle generation and UI battle stat generation.

## Targeting Skills

Targeting code should call semantic helpers, not inspect skill ids directly.

Good:

```c
GetUnitMagBy2Range(unit) + UnitHealStaffRangeBonus(unit)
```

Avoid:

```c
GetUnitMagBy2Range(unit) + UnitHasSkill(unit, SKILL_BIG_PERSONALITY)
```

This keeps skill meaning in the skill system and makes future range bonuses composable.

## Ring Skill Resolver

Keep ring resolver logic host-portable:

- Use `gba/types.h` in headers that need host tests.
- Avoid engine globals in pure resolver code.
- Pass `struct BattleUnit *` and `struct Unit *` explicitly.
- Mirror structs only when macOS host tests cannot include the real engine headers.
- If you add a new `src/engage_mechanics/*.c` object used by the ROM, add it to `ldscript.txt` sections for `.text`, `.rodata`, and `.data`.

Host tests do not prove the object is linked into the ROM.

## Tests To Add

For a new skill, add the smallest useful coverage:

- Ownership lookup: class or personal list resolves through `GetUnitSkills`.
- Dispatcher behavior: per-hit hooks fire only on their hook phase.
- Resolver behavior: deterministic ring logic works without engine globals.
- Edge cases: no skill, no target, no ring, out-of-range map positions, bounds edges.

Keep host tests pure. Do not include `global.h`, hardware headers, or engine globals in new host-portable tests.

## Build Checklist

Run:

```sh
make test
```

For ROM integration on macOS, use the repo-local command:

```sh
PATH="/opt/homebrew/opt/coreutils/libexec/gnubin:$PATH" make -j"$(nproc)" \
  FETSATOOL="python3 scripts/gfxtools/tsa_generator.py" \
  TMAP2TSA="python3 scripts/tmap2tsa.py" \
  MARTOMAP="python3 scripts/mar_to_map.py"
```

Expected outcomes:

- Matching/decomp-only work should pass `sha1sum -c checksum.sha1`.
- Feature work that changes ROM bytes should compile and link, then fail only at checksum comparison.

If the linker reports that it cannot move the location counter backwards, check EWRAM growth before moving linker symbols.

## Red Flags

Stop and rethink if you are about to:

- Add fields to `struct Unit`.
- Mutate `gBattleStats` for a deterministic forecast-visible skill.
- Read `gBmMapUnit[y][x]` without full bounds checks.
- Add a skill id without updating `SKILL_MAX`.
- Add a new source object without `ldscript.txt`.
- Put personal skill behavior directly into unrelated map, battle, or UI code.
- Make tests pass only by duplicating production logic in the test.
