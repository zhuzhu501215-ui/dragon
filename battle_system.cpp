#include "battle_system.h"

namespace Game {

int calcDamage(int rawAtk, int def) {
  int d = rawAtk - def;
  return std::max(1, d);
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

}
