// 地狱模式：三场高强度战斗，胜利解锁自定义 Boss 倍率

#include "hell_mode.h"
#include "battle_system.h"
#include "hero_system.h"
#include "monster_system.h"
#include "utils.h"

namespace Game {

bool runHellMode(GameState& g) {
  struct HellStage {
    std::string name;
    Monster m;
    double factor;
  };
  std::vector<HellStage> hs = {
      {"Hell Elite Stage I", randomElite(), 1.5},
      {"Hell Elite Stage II", randomElite(), 2.0},
      {"Hell Boss Stage", makeHellBoss(), g.customBossMultiplier},
  };
  std::cout << "\n"
            << "╔" << repeat('=', 58) << "╗\n"
            << "║ " << std::left << std::setw(56) << "Hell Mode" << " ║\n"
            << "╚" << repeat('=', 58) << "╝\n";
  std::cout << "3 stages total: Elite x1.50, Elite x2.00, Boss x" << std::fixed << std::setprecision(2)
            << g.customBossMultiplier << std::defaultfloat << "\n";
  // 进入地狱前回满生命与体力
  g.hero.hp = g.hero.maxHp;
  g.hero.stamina = g.hero.maxStamina;
  std::cout << "TEXT_46Hell ModeTEXT_47stamina .\n";
  pause();
  for (size_t i = 0; i < hs.size(); ++i) {
    Monster mm = scaledMonster(hs[i].m, hs[i].factor);
    mm.name = hs[i].name + " - " + mm.name;
    std::cout << "\n"
              << "╔" << repeat('=', 58) << "╗\n"
              << "║ " << std::left << std::setw(56) << "TEXT_48Battle " + std::to_string(i + 1) + "/3" << " ║\n"
              << "╚" << repeat('=', 58) << "╝\n";
    std::cout << "Enemy multiplier: x" << std::fixed << std::setprecision(2) << hs[i].factor
              << std::defaultfloat << "\n";
    pause();
    CombatResult cr = runBattle(g, mm);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) {
      std::cout << "\n"
                << "╔" << repeat('=', 58) << "╗\n"
                << "║ " << std::left << std::setw(56) << "Hell ModeTEXT_49" << " ║\n"
                << "╚" << repeat('=', 58) << "╝\n";
      std::cout << "TEXT_50Score: " << g.score << "  Rank: " << gradeFromScore(g.score) << "\n";
      pause();
      return false;
    }
    g.score += 180 + static_cast<int>(i) * 80;
    std::cout << "\n┌── Hero ────────────────────────────────────────────────┐\n";
    std::cout << "│ " << g.hero.name << "  ★" << g.hero.stars << "\n";
    const auto model = heroModel(g.hero.name);
    for (const auto& line : model) std::cout << "│ " << line << "\n";
    int filled = 0;
    if (g.hero.maxHp > 0) filled = static_cast<int>(std::llround((1.0 * g.hero.hp / g.hero.maxHp) * 24));
    filled = std::max(0, std::min(filled, 24));
    std::cout << "  HP [";
    for (int j = 0; j < 24; ++j) std::cout << (j < filled ? '#' : '.');
    std::cout << "] " << g.hero.hp << "/" << g.hero.maxHp << "\n";
    filled = 0;
    if (g.hero.maxStamina > 0) filled = static_cast<int>(std::llround((1.0 * g.hero.stamina / g.hero.maxStamina) * 24));
    filled = std::max(0, std::min(filled, 24));
    std::cout << "  STA [";
    for (int j = 0; j < 24; ++j) std::cout << (j < filled ? '=' : '.');
    std::cout << "] " << g.hero.stamina << "/" << g.hero.maxStamina << "\n";
    std::cout << "│ ATK " << g.hero.atk << "   DEF " << g.hero.def
              << "   LUCK " << std::fixed << std::setprecision(2) << g.hero.luck << std::defaultfloat
              << "\n";
    std::cout << "│ Skill: " << g.hero.skillName << " — " << g.hero.skillDesc << "\n";
    std::cout << "│ Equipment: ";
    if (g.hero.items.empty()) {
      std::cout << "(None)\n";
    } else {
      for (size_t j = 0; j < g.hero.items.size(); ++j) {
        std::cout << g.hero.items[j].name;
        if (j + 1 < g.hero.items.size()) std::cout << " | ";
      }
      std::cout << "\n";
    }
    if (!g.blessings.empty()) {
      std::cout << "│ Blessings: ";
      for (size_t j = 0; j < g.blessings.size(); ++j) {
        std::cout << g.blessings[j].name;
        if (j + 1 < g.blessings.size()) std::cout << ", ";
      }
      std::cout << "\n";
    }
    std::cout << "│ Gold: " << g.gold << "    Stage: " << g.stage << "/9"
              << "    Score: " << g.score << "\n";
    std::cout << "└────────────────────────────────────────────────────────┘\n";
    pause();
  }
  g.hellCleared = true;  // 三场全胜
  std::cout << "\n"
            << "╔" << repeat('=', 58) << "╗\n"
            << "║ " << std::left << std::setw(56) << "Hell ModeTEXT_51" << " ║\n"
            << "╚" << repeat('=', 58) << "╝\n";
  std::cout << "Achievement unlocked: Hell Conqueror\n";
  std::cout << "TEXT_52：TEXT_53Custom Final Boss Multiplier .\n";
  pause();
  return true;
}

// 总分阈值对应 S/A/B/C/D
char gradeFromScore(int s) {
  if (s >= 900) return 'S';
  if (s >= 650) return 'A';
  if (s >= 400) return 'B';
  if (s >= 200) return 'C';
  return 'D';
}

}
