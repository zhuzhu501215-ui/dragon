#ifndef MONSTER_SYSTEM_H
#define MONSTER_SYSTEM_H

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

Monster scaledMonster(Monster m, double factor);
Monster scaleMonsterForStage(Monster m, int stage);
double askBossMultiplier(double current);

}

#endif
