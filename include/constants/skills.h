#ifndef GUARD_CONSTANTS_SKILLS_H
#define GUARD_CONSTANTS_SKILLS_H

/* SKILL_BLOOD_FURY = 7 means this must be at least 8.
 * Maximum character skill ID — keep in sync with gSkillData[] size. */
enum { SKILL_MAX = 8 };

enum {
    SKILL_NONE            = 0,
    SKILL_SURE_SHOT       = 1,
    SKILL_ADMIRATION      = 2,
    SKILL_ALABASTER_DUTY  = 3,
    SKILL_ASPIRING_HERO   = 4,
    SKILL_BIG_PERSONALITY = 5,
    SKILL_BLINDING_FLASH  = 6,
    SKILL_BLOOD_FURY      = 7,
};

#endif // GUARD_CONSTANTS_SKILLS_H
