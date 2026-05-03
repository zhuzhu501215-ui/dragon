// Monster factories and stage stat curves.

#include "monster_system.h"

namespace Game {

Monster makeSlime(bool elite) {
  Monster m{"Slime", 80, 80, 18, 5, 12, elite};
  if (elite) {
    m.hp = m.maxHp = 160;
    m.atk = 32;
    m.def = 12;
    m.gold = 35;
    m.name = "TEXT_3·SlimeTEXT_4";
  }
  return m;
}

Monster makeGoblin(bool elite) {
  Monster m{"Goblin", 95, 95, 24, 8, 15, elite};
  if (elite) {
    m.hp = m.maxHp = 190;
    m.atk = 40;
    m.def = 16;
    m.gold = 42;
    m.name = "TEXT_3·GoblinTEXT_5";
  }
  return m;
}

Monster makeWolf(bool elite) {
  Monster m{"Wolf", 70, 70, 30, 4, 14, elite};
  if (elite) {
    m.hp = m.maxHp = 150;
    m.atk = 48;
    m.def = 10;
    m.gold = 40;
    m.name = "Elite Frost Wolf";
  }
  return m;
}

Monster makeGolem(bool elite) {
  Monster m{"Golem", 120, 120, 20, 18, 18, elite};
  if (elite) {
    m.hp = m.maxHp = 240;
    m.atk = 36;
    m.def = 28;
    m.gold = 48;
    m.name = "Elite Rock Lord";
  }
  return m;
}

// Pick one of four common (non-elite) templates.
Monster randomCommon() {
  int r = randInt(0, 3);
  if (r == 0) return makeSlime(false);
  if (r == 1) return makeGoblin(false);
  if (r == 2) return makeWolf(false);
  return makeGolem(false);
}

// Pick one of four elite templates.
Monster randomElite() {
  int r = randInt(0, 3);
  if (r == 0) return makeSlime(true);
  if (r == 1) return makeGoblin(true);
  if (r == 2) return makeWolf(true);
  return makeGolem(true);
}

// Stage boss from random elite: two lives, phase flag, bonus gold.
Monster makeStageBoss(int stage) {
  Monster b = randomElite();
  b.boss = true;
  b.lives = 2;
  b.phase = 1;
  b.name = "Boss·" + b.name;
  b.gold += 20 + stage * 2;
  return b;
}

Monster makeHellBoss() {
  Monster b{"Abyss Demon King", 320, 320, 58, 22, 160, true};
  b.boss = true;
  b.lives = 2;
  b.phase = 1;
  return b;
}

int stageHpBonus(int stage) {
  if (stage <= 1) return 0;
  if (stage <= 3) return 10 * (stage - 1);
  if (stage <= 6) return 24 + 15 * (stage - 3);
  return 69 + 20 * (stage - 6);
}

int stageAtkBonus(int stage) {
  if (stage <= 1) return 0;
  if (stage <= 3) return 2 * (stage - 1);
  if (stage <= 6) return 4 + 3 * (stage - 3);
  return 13 + 4 * (stage - 6);
}

int stageDefBonus(int stage) {
  if (stage <= 2) return 0;
  if (stage <= 6) return (stage - 2) / 2;
  return 2 + (stage - 6);
}

int goldDropMultiplierPercent(int stage, bool elite) {
  int p = 100 + (stage - 1) * 8;
  if (elite) p += 18;
  return p;
}

int stageClearGoldBonus(int stage) {
  if (stage <= 3) return 8 + stage * 2;
  if (stage <= 6) return 16 + stage * 3;
  return 26 + stage * 4;
}

// Multiply HP/ATK/DEF/gold by factor; each stat floored at 1.
Monster scaledMonster(Monster m, double factor) {
  auto s = [factor](int v) { return std::max(1, static_cast<int>(std::llround(v * factor))); };
  m.maxHp = s(m.maxHp);
  m.hp = m.maxHp;
  m.atk = s(m.atk);
  m.def = s(m.def);
  m.gold = s(m.gold);
  return m;
}

// Stack stage growth and gold percent; bosses get extra HP/ATK/DEF/gold.
Monster scaleMonsterForStage(Monster m, int stage) {
  m.maxHp += stageHpBonus(stage) + (m.elite ? stage * 4 : 0);
  m.hp = m.maxHp;
  m.atk += stageAtkBonus(stage) + (m.elite ? stage / 3 : 0);
  m.def += stageDefBonus(stage);
  m.gold = std::max(1, (m.gold * goldDropMultiplierPercent(stage, m.elite)) / 100);
  if (m.boss) {
    m.maxHp = m.maxHp * 105 / 100;
    m.hp = m.maxHp;
    m.atk += 2 + stage / 2;
    m.def += 1;
    m.gold = m.gold * 13 / 10;
  }
  return m;
}

// Read multiplier from stdin; empty line keeps current.
double askBossMultiplier(double current) {
  while (true) {
    std::cout << "\n"
              << "╔" << repeat('=', 58) << "╗\n"
              << "║ " << std::left << std::setw(56) << "Custom Final Boss Multiplier" << " ║\n"
              << "╚" << repeat('=', 58) << "╝\n";
    std::cout << "Current multiplier: x" << std::fixed << std::setprecision(2) << current << std::defaultfloat
              << "\n";
    std::cout << "Enter new multiplier [1.00 ~ 10.00], or press Enter to keep current: ";
    std::string line;
    if (!std::getline(std::cin, line)) return current;
    if (line.empty()) return current;
    try {
      double v = std::stod(line);
      if (v >= 1.0 && v <= 10.0) return v;
    } catch (...) {
    }
    std::cout << "Invalid input.\n";
    pause();
  }
}

}
