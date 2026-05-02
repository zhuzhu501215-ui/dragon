#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "utils.h"

namespace Game {

struct Equipment {
  std::string id;
  std::string name;
  std::string desc;
  int atk = 0;
  int def = 0;
  int hp = 0;
  double luck = 0.0;
  EquipSpecial special = EquipSpecial::None;
  int price = 0;

  Equipment() = default;
  Equipment(std::string id_, std::string name_, std::string desc_, int atk_, int def_,
            int hp_, double luck_, EquipSpecial special_, int price_);
};

struct Blessing {
  std::string name;
  std::string desc;
  int atk = 0;
  int def = 0;
  int hp = 0;
  double luck = 0.0;
};

struct Character {
  std::string name;
  int baseAtk = 0;
  int baseDef = 0;
  int baseMaxHp = 0;
  double baseLuck = 0.0;
  std::string skillName;
  std::string skillDesc;
  int stars = 1;
  int maxHp = 0;
  int hp = 0;
  int atk = 0;
  int def = 0;
  double luck = 0.0;
  int maxStamina = 0;
  int stamina = 0;
  std::vector<Equipment> items;

  void recomputeStats();
  bool hasBerserkerAxe() const;
};

struct Monster {
  std::string name;
  int hp = 0;
  int maxHp = 0;
  int atk = 0;
  int def = 0;
  int gold = 0;
  bool elite = false;
  bool boss = false;
  int lives = 1;
  int phase = 1;
};

struct GameState {
  Character hero;
  int stage = 1;
  int gold = 80;
  int score = 0;
  std::vector<Blessing> blessings;
  bool normalCleared = false;
  bool hellCleared = false;
  double customBossMultiplier = 3.0;
  bool tutorialMode = false;
  bool tutorialMainMenuHintShown = false;
  bool tutorialShopHintShown = false;
  bool tutorialStageHintShown = false;
  bool tutorialBossHintShown = false;
  bool tutorialPostStageHintShown = false;
};

struct CombatResult {
  bool win = false;
  int goldEarned = 0;
  std::vector<std::string> log;
};

}

#endif
