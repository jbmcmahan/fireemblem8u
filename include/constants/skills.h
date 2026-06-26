#ifndef GUARD_CONSTANTS_SKILLS_H
#define GUARD_CONSTANTS_SKILLS_H

/* Maximum character skill ID — keep in sync with gSkillData[] size.
 * SKILL_ADMIRATION = 2 means this must be at least 3.
 */
enum { SKILL_MAX = 3 };

enum {
    SKILL_NONE       = 0,
    SKILL_SURE_SHOT  = 1,
    SKILL_ADMIRATION = 2,
};

#endif // GUARD_CONSTANTS_SKILLS_H
