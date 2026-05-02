#ifndef HERO_SYSTEM_H
#define HERO_SYSTEM_H

#include "game_data.h"

namespace Game {

Equipment makeBerserkerAxe();
Character makeVaneHellion();
Character makeHero(const std::string& name, int atk, int def, int hp, double luck,
                   const std::string& skillName, const std::string& skillDesc);
std::vector<Character> makeHeroRoster();
std::vector<std::string> heroModelFrame(const std::string& heroName, int frame);
std::vector<std::string> heroModel(const std::string& heroName);
Character chooseHero();
void ensureVaneTwoStarGear(GameState& g);
void applyHeroVictoryPassive(GameState& g, std::vector<std::string>& log);

}

#endif
