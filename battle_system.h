#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "game_data.h"
#include "hero_system.h"
#include "monster_system.h"

namespace Game {

int calcDamage(int rawAtk, int def);
std::vector<std::string> monsterModel(const std::string& name);
void drawBattleVisual(const Character& heroState, const Monster& m, const std::string& action, int animFrame = 0);
void playBattleAnimFrames(const Character& heroState, const Monster& m, const std::string& action);
void parseBattleActionInput(const std::string& line, int& actionKind);
int heroAttackDamage(GameState& g, int monsterDef, bool& crit, bool& axeUsed, std::string& note);
int monsterAttack(GameState& g, Monster& m, std::string& note, bool heroDefending);
CombatResult runBattle(GameState& g, Monster m);
bool runHellMode(GameState& g);
char gradeFromScore(int s);

}

#endif
