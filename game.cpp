#include "game.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <thread>

namespace {

std::mt19937& rng() {
  static std::mt19937 gen(static_cast<unsigned>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  return gen;
}

double rand01() {
  std::uniform_real_distribution<double> d(0.0, 1.0);
  return d(rng());
}

int randInt(int a, int b) {
  std::uniform_int_distribution<int> d(a, b);
  return d(rng());
}

std::string repeat(char c, int n) { return std::string(std::max(0, n), c); }

void clearScreen() { std::cout << "\x1B[2J\x1B[H"; }

void pauseEnter() {
  std::cout << "\n[Press Enter to continue...]";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

Equipment makeBerserkerAxe() {
  Equipment e;
  e.id = "berserker_axe";
  e.name = "[Axe] Berserker Axe";
  e.desc = "Spend 20 HP before attack, then double this hit damage.";
  e.special = EquipSpecial::BerserkerAxe;
  return e;
}

const Equipment kShopCatalog[] = {
    {"wood_sword", "[Sword] Wood Sword", "ATK +5", 5, 0, 0, 0.0, EquipSpecial::None, 40},
    {"iron_armor", "[Armor] Iron Armor", "DEF +8", 0, 8, 0, 0.0, EquipSpecial::None, 55},
    {"ruby", "[Gem] Ruby", "Max HP +25", 0, 0, 25, 0.0, EquipSpecial::None, 45},
    {"clover", "[Lucky] Clover", "Luck +0.05", 0, 0, 0, 0.05, EquipSpecial::None, 50},
};

Character makeHero() {
  Character c;
  c.name = "Vane Hellion";
  c.baseAtk = 80;
  c.baseDef = 20;
  c.baseMaxHp = 150;
  c.baseLuck = 0.30;
  c.skillName = "Hell Contract";
  c.skillDesc = "At 2 stars, unlock exclusive Berserker Axe.";
  c.recomputeStats();
  c.hp = c.maxHp;
  return c;
}

Monster makeSlime(bool elite) {
  Monster m{"Slime", 80, 80, 18, 5, 12, elite};
  if (elite) m = {"Elite Slime King", 160, 160, 32, 12, 35, true};
  return m;
}
Monster makeGoblin(bool elite) {
  Monster m{"Goblin", 95, 95, 24, 8, 15, elite};
  if (elite) m = {"Elite Goblin Chief", 190, 190, 40, 16, 42, true};
  return m;
}
Monster makeWolf(bool elite) {
  Monster m{"Wolf", 70, 70, 30, 4, 14, elite};
  if (elite) m = {"Elite Frost Wolf", 150, 150, 48, 10, 40, true};
  return m;
}
Monster makeGolem(bool elite) {
  Monster m{"Golem", 120, 120, 20, 18, 18, elite};
  if (elite) m = {"Elite Rock Lord", 240, 240, 36, 28, 48, true};
  return m;
}

Monster randomCommon() {
  int r = randInt(0, 3);
  if (r == 0) return makeSlime(false);
  if (r == 1) return makeGoblin(false);
  if (r == 2) return makeWolf(false);
  return makeGolem(false);
}

Monster randomElite() {
  int r = randInt(0, 3);
  if (r == 0) return makeSlime(true);
  if (r == 1) return makeGoblin(true);
  if (r == 2) return makeWolf(true);
  return makeGolem(true);
}

int calcDamage(int rawAtk, int def) { return std::max(1, rawAtk - def); }

void ensureTwoStarGear(GameState& g) {
  if (g.hero.stars < 2) return;
  for (const auto& it : g.hero.items)
    if (it.special == EquipSpecial::BerserkerAxe) return;
  g.hero.items.push_back(makeBerserkerAxe());
  g.hero.recomputeStats();
}

int heroAttackDamage(GameState& g, int monsterDef, bool& crit, std::string& note) {
  (void)monsterDef;
  crit = false;
  note.clear();
  int base = 40;
  if (g.hero.hasBerserkerAxe() && g.hero.hp > 20) {
    g.hero.hp -= 20;
    base *= 2;
    note += "[Berserker Axe x2] ";
  }
  if (rand01() < g.hero.luck) {
    base *= 2;
    crit = true;
    note += "[Critical x2] ";
  }
  return base;
}

std::vector<std::string> heroSprite(bool attacking, bool hit, int frame) {
  bool alt = (frame % 2) == 1;
  if (hit) return alt ? std::vector<std::string>{" __/XX\\__ ", "   /||\\   ", " _/ || \\_ "}
                      : std::vector<std::string>{" __/xx\\__ ", "   /||\\   ", " _/ || \\_ "};
  if (attacking) return alt ? std::vector<std::string>{" __/OO\\==>", "   /||    ", " _/ || \\_ "}
                            : std::vector<std::string>{" __/OO\\=> ", "   /||\\   ", " _/ || \\_ "};
  return alt ? std::vector<std::string>{" __/OO\\__ ", "   /||\\   ", " _/ || \\_ "}
             : std::vector<std::string>{" __/oo\\__ ", "   /||\\   ", " _/ || \\_ "};
}

std::vector<std::string> monsterSprite(const std::string& name, bool elite, bool attacking, bool hit,
                                       int frame) {
  bool alt = (frame % 2) == 1;
  auto has = [&](const std::string& k) { return name.find(k) != std::string::npos; };
  std::string eye = hit ? "xx" : (alt ? "o*" : "oo");
  if (has("Slime")) return attacking ? std::vector<std::string>{" .------.=> ", "/(" + eye + eye + ")\\  ", "'------'    "}
                                     : std::vector<std::string>{" .------.   ", "/(" + eye + eye + ")\\  ", "'------'    "};
  if (has("Goblin")) return attacking ? std::vector<std::string>{" ,^^^^,==>  ", "/(" + eye + ")\\     ", " /_||_\\      "}
                                      : std::vector<std::string>{" ,^^^^,     ", "/(" + eye + ")\\     ", " /_||_\\      "};
  if (has("Wolf")) return attacking ? std::vector<std::string>{" /\\_/\\==>   ", "( " + eye + " )\\\\    ", " /_||_\\      "}
                                    : std::vector<std::string>{" /\\_/\\      ", "( " + eye + " )\\\\    ", " /_||_\\      "};
  if (elite) return attacking ? std::vector<std::string>{" .--------=> ", "/|" + eye + eye + "  " + eye + eye + "|\\ ", "|_==__==_|   "}
                               : std::vector<std::string>{" .--------   ", "/|" + eye + eye + "  " + eye + eye + "|\\ ", "|_==__==_|   "};
  return attacking ? std::vector<std::string>{" .------=>   ", "/|" + eye + eye + " " + eye + eye + "|\\   ", "|_==__==_|   "}
                   : std::vector<std::string>{" .------     ", "/|" + eye + eye + " " + eye + eye + "|\\   ", "|_==__==_|   "};
}

void renderHpLine(const std::string& name, int hp, int maxHp, char fill) {
  int width = 24;
  int filled = maxHp > 0 ? static_cast<int>(std::llround((1.0 * hp / maxHp) * width)) : 0;
  filled = std::clamp(filled, 0, width);
  std::cout << std::left << std::setw(16) << name << " [";
  for (int i = 0; i < width; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << std::max(0, hp) << "/" << maxHp << "\n";
}

void renderBattleVisual(const GameState& g, const Monster& m, int heroPos, int monPos,
                        const std::string& action, int round, bool heroAttacking, bool monAttacking,
                        bool heroHit, bool monHit, int frame) {
  clearScreen();
  std::cout << "======== PIXEL BATTLE ========\n";
  std::cout << "Round: " << round << "    Action: " << action << "\n\n";
  const int width = 72;
  auto h = heroSprite(heroAttacking, heroHit, frame);
  auto mm = monsterSprite(m.name, m.elite, monAttacking, monHit, frame);
  heroPos = std::clamp(heroPos, 0, width - 12);
  monPos = std::clamp(monPos, 0, width - 12);
  std::cout << "+" << repeat('-', width) << "+\n";
  for (int row = 0; row < 3; ++row) {
    std::string lane(width, ' ');
    for (size_t i = 0; i < h[row].size() && heroPos + static_cast<int>(i) < width; ++i)
      lane[heroPos + static_cast<int>(i)] = h[row][i];
    for (size_t i = 0; i < mm[row].size() && monPos + static_cast<int>(i) < width; ++i)
      lane[monPos + static_cast<int>(i)] = mm[row][i];
    std::cout << "|" << lane << "|\n";
  }
  std::cout << "+" << repeat('-', width) << "+\n\n";
  renderHpLine("Hero", g.hero.hp, g.hero.maxHp, '#');
  renderHpLine(m.name, m.hp, m.maxHp, '=');
}

char askBattleCommand() {
  while (true) {
    std::cout << "\nCommand [a=forward d=back j=attack k=guard]: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) return 'j';
    if (cmd.empty()) continue;
    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(cmd[0])));
    if (c == 'a' || c == 'd' || c == 'j' || c == 'k') return c;
    std::cout << "Invalid command.\n";
  }
}

void animateStep(const GameState& g, const Monster& m, bool heroAttack, int round, const std::string& text,
                 int heroBase, int monBase) {
  for (int i = 0; i <= 6; ++i) {
    int hp = heroAttack ? heroBase + i * 4 : heroBase;
    int mp = heroAttack ? monBase : monBase - i * 4;
    renderBattleVisual(g, m, hp, mp, text, round, heroAttack, !heroAttack, false, false, i);
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
  }
}

CombatResult runBattle(GameState& g, Monster m) {
  CombatResult r;
  r.log.push_back(">>> Encounter: " + m.name + (m.elite ? " [Elite]" : ""));
  int heroPos = 3, monPos = 60, frame = 0, round = 0;
  while (g.hero.hp > 0 && m.hp > 0) {
    ++round;
    renderBattleVisual(g, m, heroPos, monPos, "Awaiting command", round, false, false, false, false,
                       frame++);
    char action = askBattleCommand();
    if (action == 'a') heroPos = std::min(heroPos + 4, monPos - 8);
    if (action == 'd') heroPos = std::max(heroPos - 4, 0);
    if (action == 'k') renderBattleVisual(g, m, heroPos, monPos, "Guard stance", round, false, false,
                                          false, false, frame++);

    int dmg = 0;
    std::string note;
    bool crit = false, attacked = false;
    int distance = monPos - heroPos;
    if (action == 'j' && distance <= 20) {
      attacked = true;
      dmg = heroAttackDamage(g, m.def, crit, note);
      animateStep(g, m, true, round, "Hero attacks!", heroPos, monPos);
      m.hp = std::max(0, m.hp - dmg);
      renderBattleVisual(g, m, heroPos, monPos, "Hit landed!", round, false, false, false, true, frame++);
      std::this_thread::sleep_for(std::chrono::milliseconds(180));
    } else if (action == 'j') {
      note = "[Miss] Too far. Move forward first.";
    }

    std::ostringstream line;
    line << "Round " << round << ": "
         << (attacked ? ("You deal " + std::to_string(dmg) + " damage.") : "No damage dealt.")
         << (note.empty() ? "" : (" " + note));
    r.log.push_back(line.str());

    if (m.hp <= 0) {
      r.win = true;
      r.goldEarned = m.gold;
      g.gold += m.gold;
      g.score += 50 + (m.elite ? 80 : 0) + std::max(0, g.hero.hp / 2);
      break;
    }

    animateStep(g, m, false, round, m.name + " attacks!", heroPos, monPos);
    int md = calcDamage(m.atk, g.hero.def);
    if (action == 'k') md = std::max(1, md / 2);
    g.hero.hp -= md;
    r.log.push_back(m.name + " hits you for " + std::to_string(md) + ".");
    renderBattleVisual(g, m, heroPos, monPos, "You are hit!", round, false, false, true, false, frame++);
    std::this_thread::sleep_for(std::chrono::milliseconds(180));
    if (g.hero.hp <= 0) r.log.push_back("You were defeated.");
    if (monPos - heroPos > 24) monPos -= 2;
    if (monPos - heroPos < 10) monPos += 2;
  }
  clearScreen();
  return r;
}

char gradeFromScore(int s) {
  if (s >= 900) return 'S';
  if (s >= 650) return 'A';
  if (s >= 400) return 'B';
  if (s >= 200) return 'C';
  return 'D';
}

void printHeader(const std::string& t) {
  std::cout << "\n+" << repeat('=', 58) << "+\n"
            << "| " << std::left << std::setw(56) << t << " |\n"
            << "+" << repeat('=', 58) << "+\n";
}

void printBar(const std::string& title, int cur, int max, char fill, int width = 24) {
  int filled = max > 0 ? static_cast<int>(std::llround((1.0 * cur / max) * width)) : 0;
  filled = std::clamp(filled, 0, width);
  std::cout << "  " << title << " [";
  for (int i = 0; i < width; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << cur << "/" << max << "\n";
}

void printHeroPanel(const GameState& g) {
  std::cout << "\n--- HERO STATUS ---\n";
  std::cout << g.hero.name << "  *" << g.hero.stars << "\n";
  printBar("HP", g.hero.hp, g.hero.maxHp, '#');
  std::cout << "ATK " << g.hero.atk << "  DEF " << g.hero.def << "  LUCK " << std::fixed
            << std::setprecision(2) << g.hero.luck << std::defaultfloat << "\n";
  std::cout << "Skill: " << g.hero.skillName << " - " << g.hero.skillDesc << "\n";
  std::cout << "Gold: " << g.gold << "  Stage: " << g.stage << "/9  Score: " << g.score << "\n";
}

void shopLoop(GameState& g) {
  while (true) {
    printHeader("SHOP");
    printHeroPanel(g);
    std::cout << "\n[1] Buy Equipment\n[2] Buy Blessing\n[3] Upgrade to 2-star (60 gold)\n"
              << "[4] Heal +40 HP (25 gold)\n[0] Exit Shop\nChoose: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) return;
    if (cmd == "0") break;
    if (cmd == "1") {
      for (size_t i = 0; i < sizeof(kShopCatalog) / sizeof(kShopCatalog[0]); ++i) {
        const auto& e = kShopCatalog[i];
        std::cout << "[" << i + 1 << "] " << e.name << " - " << e.desc << " - " << e.price << " gold\n";
      }
      std::cout << "Pick item (0 back): ";
      std::string in;
      if (!std::getline(std::cin, in)) return;
      int id = 0;
      try { id = std::stoi(in); } catch (...) { id = 0; }
      if (id <= 0) continue;
      size_t idx = static_cast<size_t>(id - 1);
      if (idx >= sizeof(kShopCatalog) / sizeof(kShopCatalog[0])) continue;
      const auto& e = kShopCatalog[idx];
      if (g.gold < e.price) { std::cout << "Not enough gold.\n"; pauseEnter(); continue; }
      g.gold -= e.price;
      g.hero.items.push_back(e);
      g.hero.recomputeStats();
      ensureTwoStarGear(g);
      std::cout << "Purchased: " << e.name << "\n";
      pauseEnter();
    } else if (cmd == "2") {
      std::cout << "[1] War Blessing ATK+5 (55)\n[2] Guard Blessing DEF+6 (55)\n"
                << "[3] Life Blessing HP+30 (60)\n[4] Lucky Blessing LUCK+0.03 (65)\nPick: ";
      std::string in;
      if (!std::getline(std::cin, in)) return;
      Blessing b;
      int cost = 0;
      if (in == "1") { b = {"War Blessing", "ATK+5", 5, 0, 0, 0}; cost = 55; }
      else if (in == "2") { b = {"Guard Blessing", "DEF+6", 0, 6, 0, 0}; cost = 55; }
      else if (in == "3") { b = {"Life Blessing", "HP+30", 0, 0, 30, 0}; cost = 60; }
      else if (in == "4") { b = {"Lucky Blessing", "LUCK+0.03", 0, 0, 0, 0.03}; cost = 65; }
      else continue;
      if (g.gold < cost) { std::cout << "Not enough gold.\n"; pauseEnter(); continue; }
      g.gold -= cost;
      g.blessings.push_back(b);
      g.hero.baseAtk += b.atk; g.hero.baseDef += b.def; g.hero.baseMaxHp += b.hp; g.hero.baseLuck += b.luck;
      g.hero.recomputeStats();
      pauseEnter();
    } else if (cmd == "3") {
      if (g.hero.stars >= 2 || g.gold < 60) { std::cout << "Cannot upgrade now.\n"; pauseEnter(); continue; }
      g.gold -= 60;
      g.hero.stars = 2;
      ensureTwoStarGear(g);
      std::cout << "Upgrade complete.\n";
      pauseEnter();
    } else if (cmd == "4") {
      if (g.gold < 25) { std::cout << "Not enough gold.\n"; pauseEnter(); continue; }
      g.gold -= 25;
      g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 40);
      std::cout << "Healed.\n";
      pauseEnter();
    }
  }
}

bool runStage(GameState& g) {
  printHeader("Stage " + std::to_string(g.stage));
  printHeroPanel(g);
  bool hasElite = (g.stage % 3 == 0);
  std::cout << "\nThis stage has 3 battles";
  if (hasElite) std::cout << " (battle 2 is elite)";
  std::cout << ".\n";
  pauseEnter();
  for (int i = 1; i <= 3; ++i) {
    Monster m = (hasElite && i == 2) ? randomElite() : randomCommon();
    CombatResult cr = runBattle(g, m);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) return false;
    pauseEnter();
  }
  g.score += 100;
  return true;
}

}  // namespace

void Character::recomputeStats() {
  atk = baseAtk;
  def = baseDef;
  maxHp = std::min(300, baseMaxHp);
  luck = baseLuck;
  for (const auto& it : items) {
    atk += it.atk;
    def += it.def;
    maxHp = std::min(300, maxHp + it.hp);
    luck += it.luck;
  }
  hp = std::min(hp, maxHp);
}

bool Character::hasBerserkerAxe() const {
  for (const auto& it : items)
    if (it.special == EquipSpecial::BerserkerAxe) return true;
  return false;
}

void RunGame() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  GameState g;
  g.hero = makeHero();
  std::cout << "Pixel RPG (English version)\nPress Enter to start...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  while (g.stage <= 9) {
    printHeader("Main Menu");
    printHeroPanel(g);
    std::cout << "\n[1] Start Stage\n[2] Shop\n[3] Rules\n[0] Exit\nChoose: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;
    if (cmd == "0") return;
    if (cmd == "2") { shopLoop(g); continue; }
    if (cmd == "3") {
      std::cout << "HP max 300. Crit is based on luck. Stage 3/6/9 have elite in battle 2.\n";
      pauseEnter();
      continue;
    }
    if (cmd != "1") continue;
    if (!runStage(g)) {
      printHeader("Game Over");
      std::cout << "Final score: " << g.score << " Grade: " << gradeFromScore(g.score) << "\n";
      pauseEnter();
      return;
    }
    ++g.stage;
    if (g.stage > 9) {
      printHeader("Victory");
      std::cout << "Final score: " << g.score << " Grade: " << gradeFromScore(g.score) << "\n";
      pauseEnter();
      return;
    }
  }
}
