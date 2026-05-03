// Console UI: box-drawing and hero readout.

#include "console_ui.h"
#include "hero_system.h"
#include "utils.h"

namespace Game {

void printHeader(const std::string& t) {
  std::cout << "\n"
            << "╔" << repeat('=', 58) << "╗\n"
            << "║ " << std::left << std::setw(56) << t << " ║\n"
            << "╚" << repeat('=', 58) << "╝\n";
}

// Fill bar by cur/max ratio; clamp filled segment to [0, width].
void printBar(const std::string& title, int cur, int max, char fill, int width) {
  int filled = 0;
  if (max > 0) filled = static_cast<int>(std::llround((1.0 * cur / max) * width));
  filled = std::max(0, std::min(filled, width));
  std::cout << "  " << title << " [";
  for (int i = 0; i < width; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << cur << "/" << max << "\n";
}

// Read hero and global state; use heroModel() for multi-line portrait.
void printHeroPanel(const GameState& g) {
  std::cout << "\n┌── Hero ────────────────────────────────────────────────┐\n";
  std::cout << "│ " << g.hero.name << "  ★" << g.hero.stars << "\n";
  const auto model = heroModel(g.hero.name);
  for (const auto& line : model) std::cout << "│ " << line << "\n";
  printBar("HP ", g.hero.hp, g.hero.maxHp, '#', 24);
  printBar("STA ", g.hero.stamina, g.hero.maxStamina, '=', 24);
  std::cout << "│ ATK " << g.hero.atk << "   DEF " << g.hero.def
            << "   LUCK " << std::fixed << std::setprecision(2) << g.hero.luck << std::defaultfloat
            << "\n";
  std::cout << "│ Skill: " << g.hero.skillName << " — " << g.hero.skillDesc << "\n";
  std::cout << "│ Equipment: ";
  if (g.hero.items.empty()) {
    std::cout << "(None)\n";
  } else {
    for (size_t i = 0; i < g.hero.items.size(); ++i) {
      std::cout << g.hero.items[i].name;
      if (i + 1 < g.hero.items.size()) std::cout << " | ";
    }
    std::cout << "\n";
  }
  if (!g.blessings.empty()) {
    std::cout << "│ Blessings: ";
    for (size_t i = 0; i < g.blessings.size(); ++i) {
      std::cout << g.blessings[i].name;
      if (i + 1 < g.blessings.size()) std::cout << ", ";
    }
    std::cout << "\n";
  }
  std::cout << "│ Gold: " << g.gold << "    Stage: " << g.stage << "/9"
            << "    Score: " << g.score << "\n";
  std::cout << "└────────────────────────────────────────────────────────┘\n";
}

}
