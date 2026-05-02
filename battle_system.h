#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "game_data.h"
#include "hero_system.h"
#include "monster_system.h"
#include "battle_view.h"

namespace Game {

int calcDamage(int rawAtk, int def);

void parseBattleActionInput(const std::string& line, int& actionKind);

int heroAttackDamage(GameState& g, int monsterDef, bool& crit, bool& axeUsed, std::string& note);

int monsterAttack(GameState& g, Monster& m, std::string& note, bool heroDefending);

CombatResult runBattle(GameState& g, Monster m);

}

#endif
