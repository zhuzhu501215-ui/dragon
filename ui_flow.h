#ifndef UI_FLOW_H
#define UI_FLOW_H

// Game flow and tutorial copy (depends on shop, battle, hell mode).

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

// Run one full stage (three fights); returns false on defeat.
bool runStage(GameState& g);

}

#endif
