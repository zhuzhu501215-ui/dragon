#include "battle_system.h"

namespace Game {

int calcDamage(int rawAtk, int def) {
  int d = rawAtk - def;
  return std::max(1, d);
}

std::vector<std::string> monsterModel(const std::string& name) {
  if (name.find("Slime") != std::string::npos) return {"   .----.   ", " /( oo )\\  ", " \\______/"};
  if (name.find("Goblin") != std::string::npos) return {"  ,^^^^,    ", " /(><)\\    ", "  /||\\     "};
  if (name.find("Wolf") != std::string::npos || name.find("TEXT_6") != std::string::npos)
    return {" /\\_/\\      ", "( o.o )>    ", " /_ _\\      "};
  return {" .------.   ", " | [] []|   ", " |_====_|   "};
}

void drawBattleVisual(const Character& heroState, const Monster& m, const std::string& action, int animFrame) {
  clearScreen();
  auto hero = heroModelFrame(heroState.name, animFrame);
  auto mon = monsterModel(m.name);
  const int width = 68;
  const int heroPos = 4;
  const int monPos = 50;
  const int rows = std::max(hero.size(), mon.size());
  std::cout << "========== Turn-based Battle ==========\n";
  std::cout << "Action: " << action << "\n\n";
  std::cout << "+" << repeat('-', width) << "+\n";
  for (int r = 0; r < rows; ++r) {
    std::string lane(width, ' ');
    if (r < static_cast<int>(hero.size())) {
      for (size_t i = 0; i < hero[r].size() && heroPos + static_cast<int>(i) < width; ++i) {
        lane[heroPos + static_cast<int>(i)] = hero[r][i];
      }
    }
    if (r < static_cast<int>(mon.size())) {
      for (size_t i = 0; i < mon[r].size() && monPos + static_cast<int>(i) < width; ++i) {
        lane[monPos + static_cast<int>(i)] = mon[r][i];
      }
    }
    std::cout << "|" << lane << "|\n";
  }
  std::cout << "+" << repeat('-', width) << "+\n";
  std::cout << "Your HP: " << std::max(0, heroState.hp) << "/" << heroState.maxHp << "    "
            << m.name << " HP: " << std::max(0, m.hp) << "/" << m.maxHp << "\n";
  std::cout << "Stamina STA: " << std::max(0, heroState.stamina) << "/" << heroState.maxStamina << "\n";
}

void playBattleAnimFrames(const Character& heroState, const Monster& m, const std::string& action) {
  for (int f = 0; f < 3; ++f) {
    drawBattleVisual(heroState, m, action, f);
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(95));
  }
}

void parseBattleActionInput(const std::string& line, int& actionKind) {
  actionKind = 1;
  size_t i = 0;
  while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
  if (i < line.size()) {
    if (line[i] == '2')
      actionKind = 2;
    else
      actionKind = 1;
  }
}

int heroAttackDamage(GameState& g, int monsterDef, bool& crit, bool& axeUsed, std::string& note) {
  crit = false;
  axeUsed = false;
  note.clear();
  int base = calcDamage(g.hero.atk, monsterDef);
  bool hasAxe = g.hero.hasBerserkerAxe();
  if (hasAxe && g.hero.hp > 20) {
    g.hero.hp -= 20;
    base *= 2;
    axeUsed = true;
    note += "[Berserker Axe] -20 HP, damage doubled ";
  }
  if (rand01() < g.hero.luck) {
    base *= 2;
    crit = true;
    note += "[Lucky Crit] Double damage ";
  }
  return base;
}

int monsterAttack(GameState& g, Monster& m, std::string& note, bool heroDefending) {
  note.clear();
  int dmg = calcDamage(m.atk, g.hero.def);
  if (m.boss && m.phase >= 2) {
    if (rand01() < 0.28) {
      int extra = std::max(3, m.atk / 3);
      dmg += extra;
      note += "[Boss Phase 2 - Fury Smash] +" + std::to_string(extra) + " ";
    }
    if (rand01() < 0.20) {
      g.hero.hp -= 4;
      note += "[Boss Phase 2 - Burn] extra -4HP ";
    }
  }
  if (heroDefending) {
    dmg = std::max(1, dmg * 55 / 100);
    note += "[Guard] Damage taken reduced this hit ";
  }
  g.hero.hp -= dmg;
  return dmg;
}

CombatResult runBattle(GameState& g, Monster m) {
  constexpr int kAttackStaminaCost = 12;
  constexpr int kDefendStaminaRecover = 14;
  CombatResult r;
  m = scaleMonsterForStage(m, g.stage);
  std::ostringstream header;
  header << ">>> Encounter " << (m.boss ? "[Boss] " : (m.elite ? "[Elite] " : "")) << m.name << " <<<";
  r.log.push_back(header.str());
  g.hero.recomputeStats();
  g.hero.stamina = g.hero.maxStamina;
  int round = 0;
  while (g.hero.hp > 0 && m.hp > 0) {
    ++round;
    const int idleFrame = (round - 1) % 3;
    std::string inputHint;
    int actionKind = 1;
    while (true) {
      drawBattleVisual(g.hero, m, "Stage " + std::to_string(round) + " - Your Turn", idleFrame);
      if (!inputHint.empty()) std::cout << "\n>>> " << inputHint << "\n";
      std::cout << "\n┌─ Actions ─────────────────────────────────────────────────┐\n"
                << "│ [1] Attack  Cost " << kAttackStaminaCost << " stamina, deal normal attack damage                    \n"
                << "│ [2] Defend  No damage dealt; reduce incoming this turn and recover " << kDefendStaminaRecover
                << " stamina       \n"
                << "│ TEXT_19 = Attack .staminaTEXT_20Attack，TEXT_21 2 .              \n"
                << "└────────────────────────────────────────────────────────┘\n"
                << "Input 1 or 2: ";
      std::cout.flush();
      std::string cmd;
      if (!std::getline(std::cin, cmd)) return r;
      parseBattleActionInput(cmd, actionKind);
      if (actionKind == 1 && g.hero.stamina < kAttackStaminaCost) {
        std::ostringstream h;
        h << "staminaTEXT_22：AttackTEXT_23 " << kAttackStaminaCost << " points, current " << g.hero.stamina << " ."
             " TEXT_24 2 DefendTEXT_25stamina .";
        inputHint = h.str();
        continue;
      }
      break;
    }
    std::ostringstream line;
    line << "-- Stage " << round << " Round --";
    bool heroDefending = (actionKind == 2);
    bool skipAttack = (actionKind == 2);
    if (actionKind == 2) {
      int beforeSta = g.hero.stamina;
      g.hero.stamina = std::min(g.hero.maxStamina, g.hero.stamina + kDefendStaminaRecover);
      line << "\n  [TEXT_26] DefendTEXT_27：stamina " << beforeSta << " → " << g.hero.stamina;
      playBattleAnimFrames(g.hero, m, "DefendTEXT_27");
    }
    bool crit = false, axe = false;
    std::string note;
    int hdmg = 0;
    if (!skipAttack) {
      g.hero.stamina -= kAttackStaminaCost;
      hdmg = heroAttackDamage(g, m.def, crit, axe, note);
      m.hp -= hdmg;
      playBattleAnimFrames(g.hero, m, "You attack");
    }
    if (!skipAttack) {
      line << "\n  [#] You dealt " << hdmg << " damage";
      line << "  (stamina " << g.hero.stamina << "/" << g.hero.maxStamina << ")";
      if (!note.empty()) line << "  " << note;
    }
    if (m.hp < 0) m.hp = 0;
    int bar = m.maxHp > 0 ? (20 * m.hp / m.maxHp) : 0;
    bar = std::max(0, std::min(bar, 20));
    line << "\n  [" << repeat('#', bar) << repeat('.', 20 - bar) << "] "
         << m.hp << "/" << m.maxHp;
    r.log.push_back(line.str());
    if (m.hp <= 0) {
      if (m.boss && m.lives > 1) {
        --m.lives;
        m.phase = 2;
        m.maxHp = std::max(1, m.maxHp * 60 / 100);
        m.hp = m.maxHp;
        m.atk += 5 + g.stage;
        m.def += 2 + g.stage / 3;
        std::ostringstream phaseShift;
        phaseShift << "  [!!] " << m.name
                   << " TEXT_28：TEXT_29，TEXT_30DefendTEXT_31！";
        r.log.push_back(phaseShift.str());
        drawBattleVisual(g.hero, m, "Boss enters Phase 2", idleFrame);
        continue;
      }
      r.win = true;
      r.goldEarned = m.gold;
      std::ostringstream win;
      win << "\n  [+] Defeated! Gold +" << m.gold;
      r.log.push_back(win.str());
      g.gold += m.gold;
      g.score += 50 + (m.elite ? 80 : 0) + std::max(0, g.hero.hp / 2);
      applyHeroVictoryPassive(g, r.log);
      break;
    }
    std::string mnote;
    int md = monsterAttack(g, m, mnote, heroDefending);
    drawBattleVisual(g.hero, m, m.name + " counterattacks", (round + 1) % 3);
    std::ostringstream defl;
    defl << "  [*] " << m.name << " counterattacks " << md << " -> Your remaining HP "
         << std::max(0, g.hero.hp) << "/" << g.hero.maxHp << "  stamina "
         << g.hero.stamina << "/" << g.hero.maxStamina;
    if (!mnote.empty()) defl << "  " << mnote;
    r.log.push_back(defl.str());
    if (g.hero.hp <= 0) {
      r.win = false;
      r.log.push_back("\n  [X] You have fallen...");
      break;
    }
  }
  g.hero.stamina = g.hero.maxStamina;
  return r;
}

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
    for (int i = 0; i < 24; ++i) std::cout << (i < filled ? '#' : '.');
    std::cout << "] " << g.hero.hp << "/" << g.hero.maxHp << "\n";
    filled = 0;
    if (g.hero.maxStamina > 0) filled = static_cast<int>(std::llround((1.0 * g.hero.stamina / g.hero.maxStamina) * 24));
    filled = std::max(0, std::min(filled, 24));
    std::cout << "  STA [";
    for (int i = 0; i < 24; ++i) std::cout << (i < filled ? '=' : '.');
    std::cout << "] " << g.hero.stamina << "/" << g.hero.maxStamina << "\n";
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
    pause();
  }
  g.hellCleared = true;
  std::cout << "\n"
            << "╔" << repeat('=', 58) << "╗\n"
            << "║ " << std::left << std::setw(56) << "Hell ModeTEXT_51" << " ║\n"
            << "╚" << repeat('=', 58) << "╝\n";
  std::cout << "Achievement unlocked: Hell Conqueror\n";
  std::cout << "TEXT_52：TEXT_53Custom Final Boss Multiplier .\n";
  pause();
  return true;
}

char gradeFromScore(int s) {
  if (s >= 900) return 'S';
  if (s >= 650) return 'A';
  if (s >= 400) return 'B';
  if (s >= 200) return 'C';
  return 'D';
}

}
