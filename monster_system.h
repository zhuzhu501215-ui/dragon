#ifndef MONSTER_SYSTEM_H
#define MONSTER_SYSTEM_H

// Monster factories, stage scaling, gold multipliers, clear bonuses.

#include "game_data.h"

namespace Game {

Monster makeSlime(bool elite);
Monster makeGoblin(bool elite);
Monster makeWolf(bool elite);
Monster makeGolem(bool elite);
Monster randomCommon();
Monster randomElite();
Monster makeStageBoss(int stage);
Monster makeHellBoss();

int stageHpBonus(int stage);
int stageAtkBonus(int stage);
int stageDefBonus(int stage);
int goldDropMultiplierPercent(int stage, bool elite);
int stageClearGoldBonus(int stage);

// Scale all combat stats and gold by factor (e.g. hell mode).
Monster scaledMonster(Monster m, double factor);
// Add stage-based HP/ATK/DEF/gold; bosses get extra bumps.
Monster scaleMonsterForStage(Monster m, int stage);
// Prompt for final boss multiplier in [1, 10]; retry on bad input.
double askBossMultiplier(double current);

}

#endif
