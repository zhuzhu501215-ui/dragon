#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>

enum class EquipSpecial { None, BerserkerAxe };

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
};

struct CombatResult {
  bool win = false;
  int goldEarned = 0;
  std::vector<std::string> log;
};

struct GameState {
  Character hero;
  int stage = 1;
  int gold = 80;
  int score = 0;
  std::vector<Blessing> blessings;
};

void RunGame();

#endif
