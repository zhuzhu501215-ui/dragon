#ifndef MONSTER_SYSTEM_H
#define MONSTER_SYSTEM_H

// 怪物生成、按关卡缩放属性、金币倍率与通关金币奖励

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

// 全体数值乘以 factor（地狱模式等）
Monster scaledMonster(Monster m, double factor);
// 按关卡追加 HP/攻防与金币倍率，Boss 再额外强化
Monster scaleMonsterForStage(Monster m, int stage);
// 交互输入最终 Boss 倍率 [1, 10]，非法则重试
double askBossMultiplier(double current);

}

#endif
