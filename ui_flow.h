#ifndef UI_FLOW_H
#define UI_FLOW_H

// 游戏流程与教程提示文案（依赖商店、战斗、地狱模式等子系统）

#include "console_ui.h"
#include "shop_system.h"
#include "battle_system.h"
#include "hell_mode.h"

namespace Game {

void mainMenu();

void printStartScreen();

void printEndingScreen(bool win, const GameState& g);

void printTutorialIntro();

void printTutorialMainMenuHint(GameState& g);

void printTutorialShopHint(GameState& g);

void printTutorialStageHint(GameState& g, bool hasElite);

void printTutorialPostStageHint(GameState& g);

// 执行一整关：三场战斗，失败则返回 false
bool runStage(GameState& g);

}

#endif
