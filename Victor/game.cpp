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

void shopLoop(GameState& g);

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

constexpr int kSpriteRows = 13;
constexpr int kHeroSpriteW = 36;
constexpr int kMonsterSpriteW = 40;
constexpr int kLaneW = 92;

std::string spriteRow(std::string s, int w) {
  if (static_cast<int>(s.size()) > w) return s.substr(0, w);
  return s + std::string(w - s.size(), ' ');
}

std::vector<std::string> padSpriteHeight(std::vector<std::string> core, int w, int h) {
  for (auto& L : core) L = spriteRow(L, w);
  int top = std::max(0, (h - static_cast<int>(core.size())) / 2);
  std::vector<std::string> out;
  for (int i = 0; i < top; ++i) out.push_back(std::string(w, ' '));
  out.insert(out.end(), core.begin(), core.end());
  while (static_cast<int>(out.size()) < h) out.push_back(std::string(w, ' '));
  out.resize(static_cast<size_t>(h));
  return out;
}

void clearScreen() { std::cout << "\x1B[2J\x1B[H"; }

void pauseEnter() {
  std::cout << "\n[ 按 Enter 继续... ]";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void pauseEnterOrShop(GameState& g) {
  while (true) {
    std::cout << "\n[Enter] 继续   [s] 打开商店: ";
    std::string line;
    if (!std::getline(std::cin, line)) return;
    if (!line.empty() && (line[0] == 's' || line[0] == 'S'))
      shopLoop(g);
    else
      break;
  }
}

Equipment makeBerserkerAxe() {
  Equipment e;
  e.id = "berserker_axe";
  e.name = "[斧] 狂战士之斧";
  e.desc = "攻击前若 HP>20 则消耗 20 HP，本击伤害翻倍";
  e.special = EquipSpecial::BerserkerAxe;
  return e;
}

const Equipment kShopCatalog[] = {
    {"wood_sword", "[剑] 木剑", "攻击力 +5", 5, 0, 0, 0.0, EquipSpecial::None, 40},
    {"iron_armor", "[甲] 铁甲", "防御力 +8", 0, 8, 0, 0.0, EquipSpecial::None, 55},
    {"ruby", "[宝] 红宝石", "最大生命 +25", 0, 0, 25, 0.0, EquipSpecial::None, 45},
    {"clover", "[幸] 四叶草", "幸运 +0.05", 0, 0, 0, 0.05, EquipSpecial::None, 50},
    {"long_bow", "[弓] 猎手长弓", "按 r 远程攻击，无视距离", 0, 0, 0, 0.0, EquipSpecial::LongBow, 55},
};

Character makeHero() {
  Character c;
  c.name = "范·地狱火";
  c.baseAtk = 50;
  c.baseDef = 18;
  c.baseMaxHp = 160;
  c.baseLuck = 0.18;
  c.skillName = "地狱契约";
  c.skillDesc = "升至二星后获得专属「狂战士之斧」。";
  c.recomputeStats();
  c.hp = c.maxHp;
  return c;
}

Monster makeSlime(bool elite) {
  // 普通怪：4 刀内死（无暴击）≈ hp <= 4*(50-def)；弓 10 箭内 ≈ hp <= 10*(33-def/2)
  Monster m{"史莱姆", 156, 156, 14, 9, 14, elite};
  if (elite) m = {"精英·史莱姆王", 400, 400, 26, 18, 38, true};
  return m;
}
Monster makeGoblin(bool elite) {
  Monster m{"哥布林", 152, 152, 18, 11, 16, elite};
  if (elite) m = {"精英·哥布林酋长", 460, 460, 32, 22, 44, true};
  return m;
}
Monster makeWolf(bool elite) {
  Monster m{"野狼", 168, 168, 24, 7, 15, elite};
  if (elite) m = {"精英·霜狼", 420, 420, 38, 14, 42, true};
  return m;
}
Monster makeGolem(bool elite) {
  Monster m{"石像怪", 124, 124, 16, 18, 20, elite};
  if (elite) m = {"精英·岩石领主", 520, 520, 28, 40, 52, true};
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

int heroMeleeDamage(GameState& g, int monsterDef, bool& crit, std::string& note) {
  crit = false;
  note.clear();
  int base = calcDamage(g.hero.atk, monsterDef);
  if (g.hero.hasBerserkerAxe() && g.hero.hp > 20) {
    g.hero.hp -= 20;
    base *= 2;
    note += "[狂战士之斧] 本击翻倍 ";
  }
  if (rand01() < g.hero.luck) {
    base *= 2;
    crit = true;
    note += "[暴击] 双倍 ";
  }
  return base;
}

int heroRangedDamage(GameState& g, int monsterDef, bool& crit, std::string& note) {
  crit = false;
  note.clear();
  int effAtk = static_cast<int>(std::llround(g.hero.atk * 0.65));
  int halfDef = monsterDef / 2;
  int base = std::max(1, effAtk - halfDef);
  if (g.hero.hasBerserkerAxe() && g.hero.hp > 20) {
    g.hero.hp -= 20;
    base *= 2;
    note += "[狂战士之斧] 本击翻倍 ";
  }
  if (rand01() < g.hero.luck) {
    base *= 2;
    crit = true;
    note += "[暴击] 双倍 ";
  }
  note += "[弓箭] 无视距离 ";
  return base;
}

enum class HeroPose { Idle, Melee, Ranged, Hit };

std::vector<std::string> heroSprite(HeroPose pose, int frame) {
  bool f = (frame % 2) == 1;
  auto coreIdle = [] {
    return std::vector<std::string>{
        "                                    ",
        "           *    HERO    *           ",
        "              ._______.             ",
        "             /   O O   \\            ",
        "            |    \\_/    |           ",
        "            |===========|           ",
        "           /|+++++++++++|\\          ",
        "          /_|+++++++++++|_\\         ",
        "            /|    |    |\\           ",
        "           / |    |    | \\          ",
        "          /__|____|____|__\\         ",
        "            /        \\              ",
        "           ''        ''             "};
  };
  auto coreMelee = [f] {
    return std::vector<std::string>{
        "                                    ",
        "           *   SLASH   *            ",
        "              ._______.             ",
        "             /   O O   \\            ",
        "            |    \\_/    |           ",
        "            |===========|           ",
        f ? "           /+++++++++++\\===>      "
          : "           |+++++++++++|===>      ",
        "          /_|+++++++++++|_\\         ",
        "            /|    |    |\\           ",
        "           / |    |    | \\          ",
        "          /__|____|____|__\\         ",
        "            /        \\              ",
        "           ''        ''             "};
  };
  auto coreRanged = [f] {
    return std::vector<std::string>{
        "                                    ",
        "           *    BOW    *            ",
        "              ._______.             ",
        "             /   O O   \\            ",
        "            |    \\_/    |           ",
        "            |===========|           ",
        f ? "           |+++++}~~~~~>           "
          : "           |+++++}====>          ",
        "          /_|+++++++++++|_\\         ",
        "            /|    |    |\\           ",
        "           / |    |    | \\          ",
        "          /__|____|____|__\\         ",
        "            /        \\              ",
        "           ''        ''             "};
  };
  auto coreHit = [] {
    return std::vector<std::string>{
        "                                    ",
        "           *   OUCH!   *            ",
        "              ._______.             ",
        "             /   X X   \\            ",
        "            |    \\_/    |           ",
        "            |===========|           ",
        "           /|+++++++++++|\\          ",
        "          /_|+++++++++++|_\\         ",
        "            /|    |    |\\           ",
        "           / |    |    | \\          ",
        "          /__|____|____|__\\         ",
        "            /        \\              ",
        "           ''        ''             "};
  };
  switch (pose) {
    case HeroPose::Hit:
      return padSpriteHeight(coreHit(), kHeroSpriteW, kSpriteRows);
    case HeroPose::Melee:
      return padSpriteHeight(coreMelee(), kHeroSpriteW, kSpriteRows);
    case HeroPose::Ranged:
      return padSpriteHeight(coreRanged(), kHeroSpriteW, kSpriteRows);
    default:
      return padSpriteHeight(coreIdle(), kHeroSpriteW, kSpriteRows);
  }
}

std::vector<std::string> monsterSprite(const std::string& name, bool elite, bool attacking, bool hit,
                                       int frame) {
  bool f = (frame % 2) == 1;
  auto has = [&](const char* k) { return name.find(k) != std::string::npos; };
  std::string L = hit ? "X" : (f ? "@" : "o");
  std::string R = hit ? "X" : "o";

  auto slime = [&](bool big, bool atk, bool h) {
    std::string tag = big ? "   [ELITE SLIME]   " : "    [SLIME]    ";
    std::string eyeL = h ? "XX" : L + L;
    std::string eyeR = h ? "XX" : R + R;
    std::string mouth = atk ? "><><" : "~~~~";
    return std::vector<std::string>{
        "                                        ",
        "           ~  o O o  ~                  ",
        tag,
        "       .==================.             ",
        "      /                    \\            ",
        "     |  .----------------.  |           ",
        "     | /  (" + eyeL + "    " + eyeR + ")  \\ |           ",
        "     | |    (" + mouth + ")    | |           ",
        "     |  \\______vv______/  |           ",
        "      \\__________________/            ",
        "       ''''''''''''''''''             ",
        "          ~~~~~~~~~~~~                ",
        "                                        "};
  };

  auto goblin = [&](bool big, bool atk, bool h) {
    std::string tag = big ? "  [ELITE GOBLIN]  " : "   [GOBLIN]   ";
    std::string e1 = h ? "XX" : L + "o";
    std::string e2 = h ? "XX" : "o" + R;
    std::string arm = atk ? "===|STAB|==>" : "   |dagger|   ";
    return std::vector<std::string>{
        "                                        ",
        "         *  camp  *                   ",
        tag,
        "        ,,,,,,,,,,,,,,,               ",
        "       /^^^^^^^^^^^^^^^\\              ",
        "      |  /\\         /\\  |             ",
        "      | | " + e1 + "  ><  " + e2 + " | |             ",
        "      | |   /####\\   | |             ",
        "      |  \\_/######\\_/  |             ",
        "      |________________|             ",
        "         / " + arm + " \\            ",
        "        /______vv______\\             ",
        "                                        "};
  };

  auto wolf = [&](bool big, bool atk, bool h) {
    std::string tag = big ? "   [ELITE WOLF]   " : "     [WOLF]     ";
    std::string e1 = h ? "XX" : L + "~";
    std::string e2 = h ? "XX" : "~" + R;
    std::string jaw = atk ? ">>> //// >>>" : "   ////     ";
    return std::vector<std::string>{
        "                                        ",
        "       *  cold wind  *                 ",
        tag,
        "      /\\___/\\____/\\___/\\             ",
        "     /  \\__/  \\__/  \\__/ \\            ",
        "    |    ><  ~~~~  ><    |            ",
        "    |   /||\\ /||||\\ /||\\   |            ",
        "    |__/||||||||||||||\\__|            ",
        "    | " + e1 + " \\_||||||_/ " + e2 + " |            ",
        "     \\_____\\||||/_____/             ",
        "           " + jaw + "               ",
        "         vv    vv    vv               ",
        "                                        "};
  };

  auto stone = [&](bool big, bool atk, bool h) {
    std::string tag = big ? " [ELITE GOLEM] " : "   [GOLEM]   ";
    std::string e1 = h ? "XX" : L + "o";
    std::string e2 = h ? "XX" : "o" + R;
    std::string rune = atk ? ">> GLOW <<" : "  .......  ";
    return std::vector<std::string>{
        "                                        ",
        "        .==============.               ",
        "       /| " + rune + " |\\              ",
        "      | |##############| |             ",
        "      | |##[" + e1 + "][" + e2 + "]##| |             ",
        "      | |## \\====/ ##| |             ",
        "      | |###||||||||###| |             ",
        "      | |___########___| |             ",
        "      |/_____||||||_____\\|             ",
        "       \\_____||||||_____/             ",
        tag,
        "                                        ",
        "                                        "};
  };

  if (has("史莱姆")) return padSpriteHeight(slime(elite, attacking, hit), kMonsterSpriteW, kSpriteRows);
  if (has("哥布林")) return padSpriteHeight(goblin(elite, attacking, hit), kMonsterSpriteW, kSpriteRows);
  if (has("狼")) return padSpriteHeight(wolf(elite, attacking, hit), kMonsterSpriteW, kSpriteRows);
  if (has("石像") || has("岩石"))
    return padSpriteHeight(stone(elite, attacking, hit), kMonsterSpriteW, kSpriteRows);
  return padSpriteHeight(stone(false, attacking, hit), kMonsterSpriteW, kSpriteRows);
}

int spriteWidth(const std::vector<std::string>& s) {
  int w = 0;
  for (const auto& line : s) w = std::max(w, static_cast<int>(line.size()));
  return w;
}

void renderHpLine(const std::string& name, int hp, int maxHp, char fill) {
  int width = 24;
  int filled = maxHp > 0 ? static_cast<int>(std::llround((1.0 * hp / maxHp) * width)) : 0;
  filled = std::clamp(filled, 0, width);
  std::cout << std::left << std::setw(18) << name << " [";
  for (int i = 0; i < width; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << std::max(0, hp) << "/" << maxHp << "\n";
}

void renderBattleVisual(const GameState& g, const Monster& m, int heroPos, int monPos,
                        const std::string& action, int round, HeroPose heroPose, bool monAttacking,
                        bool monHit, int frame, const std::string& arrowLine = "") {
  clearScreen();
  std::cout << "======== 像素风战斗 ========\n";
  std::cout << "回合 " << round << "    " << action << "\n\n";
  const int width = kLaneW;
  auto h = heroSprite(heroPose, frame);
  auto mm = monsterSprite(m.name, m.elite, monAttacking, monHit, frame);
  int hw = spriteWidth(h);
  int mw = spriteWidth(mm);
  heroPos = std::clamp(heroPos, 0, std::max(0, width - hw - 1));
  monPos = std::clamp(monPos, 0, std::max(0, width - mw - 1));

  if (!arrowLine.empty()) std::cout << arrowLine << "\n\n";

  std::cout << "+" << repeat('-', width) << "+\n";
  for (int row = 0; row < kSpriteRows; ++row) {
    std::string lane(width, ' ');
    if (row < static_cast<int>(h.size())) {
      for (size_t i = 0; i < h[row].size() && heroPos + static_cast<int>(i) < width; ++i)
        lane[heroPos + static_cast<int>(i)] = h[row][i];
    }
    if (row < static_cast<int>(mm.size())) {
      for (size_t i = 0; i < mm[row].size() && monPos + static_cast<int>(i) < width; ++i)
        lane[monPos + static_cast<int>(i)] = mm[row][i];
    }
    std::cout << "|" << lane << "|\n";
  }
  std::cout << "+" << repeat('-', width) << "+\n\n";
  renderHpLine("主角", g.hero.hp, g.hero.maxHp, '#');
  renderHpLine(m.name, m.hp, m.maxHp, '=');
}

void animateMelee(const GameState& g, const Monster& m, int round, const std::string& text, int h0,
                  int m0) {
  for (int i = 0; i <= 5; ++i) {
    int hp = h0 + i * 3;
    int mp = m0;
    renderBattleVisual(g, m, hp, mp, text, round, HeroPose::Melee, true, false, i);
    std::this_thread::sleep_for(std::chrono::milliseconds(65));
  }
  renderBattleVisual(g, m, h0, m0, text, round, HeroPose::Idle, false, false, 0);
}

void animateMonsterRush(const GameState& g, const Monster& m, int round, const std::string& text, int h0,
                        int m0) {
  for (int i = 0; i <= 5; ++i) {
    int hp = h0;
    int mp = m0 - i * 3;
    renderBattleVisual(g, m, hp, mp, text, round, HeroPose::Idle, true, false, i);
    std::this_thread::sleep_for(std::chrono::milliseconds(65));
  }
  renderBattleVisual(g, m, h0, m0, text, round, HeroPose::Idle, false, false, 0);
}

void animateArrowFixed(const GameState& g, const Monster& m, int round, int h0, int m0) {
  const int laneW = kLaneW;
  for (int step = 0; step <= 10; ++step) {
    std::string line(laneW, ' ');
    int start = h0 + kHeroSpriteW / 2;
    int target = std::min(m0 + 4, laneW - 2);
    int pos = start + (target - start) * step / 10;
    for (int p = start; p <= pos && p < static_cast<int>(line.size()); ++p) line[p] = (p == pos ? '>' : '-');
    renderBattleVisual(g, m, h0, m0, "放箭", round, HeroPose::Ranged, false, false, step, "箭轨: " + line);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
}

char askBattleCommand(bool hasBow) {
  while (true) {
    std::cout << "\n操作 [a 前进 d 后退 j 近战 k 防御";
    std::cout << " r 弓箭(需长弓)";
    std::cout << " s 商店(不消耗回合)";
    std::cout << "]: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) return 'j';
    if (cmd.empty()) continue;
    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(cmd[0])));
    if (c == 's') return 's';
    if (c == 'a' || c == 'd' || c == 'j' || c == 'k') return c;
    if (c == 'r') {
      if (!hasBow) {
        std::cout << "未装备长弓，请在商店购买「猎手长弓」。\n";
        continue;
      }
      return 'r';
    }
    std::cout << "无效输入。\n";
  }
}

CombatResult runBattle(GameState& g, Monster m) {
  CombatResult r;
  r.log.push_back(">>> 遭遇 " + std::string(m.elite ? "[精英] " : "") + m.name);
  const int meleeReach = 22;
  int heroPos = 2, monPos = 54, frame = 0, round = 0;
  const bool bow = g.hero.hasLongBow();

  while (g.hero.hp > 0 && m.hp > 0) {
    ++round;
    renderBattleVisual(g, m, heroPos, monPos, "等待指令", round, HeroPose::Idle, false, false, frame++);

    char action = askBattleCommand(bow);
    if (action == 's') {
      --round;
      shopLoop(g);
      continue;
    }
    if (action == 'a') {
      heroPos = std::min(heroPos + 3, monPos - 10);
      renderBattleVisual(g, m, heroPos, monPos, "前进", round, HeroPose::Idle, false, false, frame++);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else if (action == 'd') {
      heroPos = std::max(heroPos - 3, 0);
      renderBattleVisual(g, m, heroPos, monPos, "后退", round, HeroPose::Idle, false, false, frame++);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else if (action == 'k') {
      renderBattleVisual(g, m, heroPos, monPos, "防御姿态", round, HeroPose::Idle, false, false, frame++);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int dmg = 0;
    std::string note;
    bool crit = false;
    bool dealt = false;
    int dist = monPos - heroPos;

    if (action == 'j') {
      if (dist <= meleeReach) {
        dealt = true;
        dmg = heroMeleeDamage(g, m.def, crit, note);
        animateMelee(g, m, round, "近战挥砍", heroPos, monPos);
        m.hp = std::max(0, m.hp - dmg);
        renderBattleVisual(g, m, heroPos, monPos, "命中", round, HeroPose::Idle, false, true, frame++);
        std::this_thread::sleep_for(std::chrono::milliseconds(160));
      } else {
        note = "[够不着] 请前进或使用弓箭。";
      }
    } else if (action == 'r' && bow) {
      dealt = true;
      dmg = heroRangedDamage(g, m.def, crit, note);
      animateArrowFixed(g, m, round, heroPos, monPos);
      m.hp = std::max(0, m.hp - dmg);
      renderBattleVisual(g, m, heroPos, monPos, "箭矢命中", round, HeroPose::Idle, false, true, frame++);
      std::this_thread::sleep_for(std::chrono::milliseconds(160));
    }

    std::ostringstream line;
    line << "-- 第 " << round << " 回合 -- ";
    if (dealt)
      line << "造成 " << dmg << " 伤害";
    else if (action == 'k')
      line << "本回合防御";
    else
      line << "未造成伤害";
    if (!note.empty()) line << "  " << note;
    r.log.push_back(line.str());

    if (m.hp <= 0) {
      r.win = true;
      r.goldEarned = m.gold;
      g.gold += m.gold;
      g.score += 50 + (m.elite ? 80 : 0) + std::max(0, g.hero.hp / 2);
      r.log.push_back("击败！获得金币 +" + std::to_string(m.gold));
      break;
    }

    animateMonsterRush(g, m, round, m.name + " 反击", heroPos, monPos);
    int md = calcDamage(m.atk, g.hero.def);
    if (action == 'k') md = std::max(1, md / 2);
    g.hero.hp -= md;
    r.log.push_back(m.name + " 对你造成 " + std::to_string(md) + " 伤害" +
                    (action == 'k' ? "（防御减半）" : ""));
    renderBattleVisual(g, m, heroPos, monPos, "受到伤害", round, HeroPose::Hit, false, false, frame++);
    std::this_thread::sleep_for(std::chrono::milliseconds(160));
    if (g.hero.hp <= 0) {
      r.win = false;
      r.log.push_back("你倒下了...");
      break;
    }
    if (dist > 28) monPos -= 1;
    if (dist < 12) monPos += 1;
    monPos = std::clamp(monPos, 48, 78);
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

void printBar(const std::string& title, int cur, int max, char fill, int w = 24) {
  int filled = max > 0 ? static_cast<int>(std::llround((1.0 * cur / max) * w)) : 0;
  filled = std::clamp(filled, 0, w);
  std::cout << "  " << title << " [";
  for (int i = 0; i < w; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << cur << "/" << max << "\n";
}

void printHeroPanel(const GameState& g) {
  std::cout << "\n--- 角色状态 ---\n";
  std::cout << g.hero.name << "  ★" << g.hero.stars;
  if (g.hero.hasLongBow()) std::cout << "  [已装备长弓: 战斗中按 r 远程]";
  std::cout << "\n";
  printBar("HP", g.hero.hp, g.hero.maxHp, '#');
  std::cout << "ATK " << g.hero.atk << "  DEF " << g.hero.def << "  LUCK " << std::fixed
            << std::setprecision(2) << g.hero.luck << std::defaultfloat << "\n";
  std::cout << "技能: " << g.hero.skillName << " — " << g.hero.skillDesc << "\n";
  std::cout << "金币: " << g.gold << "  关卡: " << g.stage << "/9  积分: " << g.score << "\n";
}

void shopLoop(GameState& g) {
  while (true) {
    printHeader("商 城");
    printHeroPanel(g);
    std::cout << "\n[1] 购买装备\n[2] 购买赐福\n[3] 升星至二星 (60 金币，解锁狂战士之斧)\n"
              << "[4] 回复 40 HP (25 金币)\n[0] 离开\n请选择: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) return;
    if (cmd == "0") break;
    if (cmd == "1") {
      for (size_t i = 0; i < sizeof(kShopCatalog) / sizeof(kShopCatalog[0]); ++i) {
        const auto& e = kShopCatalog[i];
        std::cout << "[" << i + 1 << "] " << e.name << " — " << e.desc << " — " << e.price << " 金币\n";
      }
      std::cout << "输入编号购买，0 返回: ";
      std::string in;
      if (!std::getline(std::cin, in)) return;
      int id = 0;
      try { id = std::stoi(in); } catch (...) { id = 0; }
      if (id <= 0) continue;
      size_t idx = static_cast<size_t>(id - 1);
      if (idx >= sizeof(kShopCatalog) / sizeof(kShopCatalog[0])) continue;
      const auto& e = kShopCatalog[idx];
      if (g.gold < e.price) { std::cout << "金币不足。\n"; pauseEnter(); continue; }
      bool skip = false;
      if (e.special == EquipSpecial::LongBow) {
        for (const auto& it : g.hero.items)
          if (it.special == EquipSpecial::LongBow) {
            std::cout << "已拥有长弓。\n";
            pauseEnter();
            skip = true;
            break;
          }
      }
      if (skip) continue;
      g.gold -= e.price;
      g.hero.items.push_back(e);
      g.hero.recomputeStats();
      ensureTwoStarGear(g);
      std::cout << "已购买 " << e.name << "\n";
      pauseEnter();
    } else if (cmd == "2") {
      std::cout << "[1] 战神祝福 ATK+5 (55)\n[2] 守护祝福 DEF+6 (55)\n[3] 生命祝福 HP+30 (60)\n"
                << "[4] 幸运祝福 LUCK+0.03 (65)\n选: ";
      std::string in;
      if (!std::getline(std::cin, in)) return;
      Blessing b;
      int cost = 0;
      if (in == "1") { b = {"战神祝福", "ATK+5", 5, 0, 0, 0}; cost = 55; }
      else if (in == "2") { b = {"守护祝福", "DEF+6", 0, 6, 0, 0}; cost = 55; }
      else if (in == "3") { b = {"生命祝福", "HP+30", 0, 0, 30, 0}; cost = 60; }
      else if (in == "4") { b = {"幸运祝福", "LUCK+0.03", 0, 0, 0, 0.03}; cost = 65; }
      else continue;
      if (g.gold < cost) { std::cout << "金币不足。\n"; pauseEnter(); continue; }
      g.gold -= cost;
      g.blessings.push_back(b);
      g.hero.baseAtk += b.atk;
      g.hero.baseDef += b.def;
      g.hero.baseMaxHp += b.hp;
      g.hero.baseLuck += b.luck;
      g.hero.recomputeStats();
      if (b.hp > 0) g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + b.hp);
      ensureTwoStarGear(g);
      std::cout << "获得 " << b.name << "\n";
      pauseEnter();
    } else if (cmd == "3") {
      if (g.hero.stars >= 2 || g.gold < 60) { std::cout << "无法升星。\n"; pauseEnter(); continue; }
      g.gold -= 60;
      g.hero.stars = 2;
      ensureTwoStarGear(g);
      std::cout << "升星成功。\n";
      pauseEnter();
    } else if (cmd == "4") {
      if (g.gold < 25) { std::cout << "金币不足。\n"; pauseEnter(); continue; }
      g.gold -= 25;
      g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 40);
      std::cout << "已回复。\n";
      pauseEnter();
    }
  }
}

bool runStage(GameState& g) {
  printHeader("第 " + std::to_string(g.stage) + " 关");
  printHeroPanel(g);
  bool hasElite = (g.stage % 3 == 0);
  std::cout << "\n本关共 3 场战斗";
  if (hasElite) std::cout << "（第 2 场为精英）";
  std::cout << "。\n";
  pauseEnterOrShop(g);
  for (int i = 1; i <= 3; ++i) {
    Monster mon = (hasElite && i == 2) ? randomElite() : randomCommon();
    CombatResult cr = runBattle(g, mon);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) return false;
    printHeroPanel(g);
    pauseEnterOrShop(g);
  }
  g.score += 100;
  std::cout << "\n*** 第 " << g.stage << " 关通关！*** 评级参考: " << gradeFromScore(g.score) << "\n";
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

bool Character::hasLongBow() const {
  for (const auto& it : items)
    if (it.special == EquipSpecial::LongBow) return true;
  return false;
}

void RunGame() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  GameState g;
  g.hero = makeHero();
  std::cout << "像素风控制台 RPG（中文版）\n按 Enter 开始...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  while (g.stage <= 9) {
    printHeader("主菜单 — 第 " + std::to_string(g.stage) + " 关前");
    printHeroPanel(g);
    std::cout << "\n[1] 进入战斗\n[2] 商店\n[3] 规则说明\n[0] 退出\n请选择: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;
    if (cmd == "0") return;
    if (cmd == "2") { shopLoop(g); continue; }
    if (cmd == "3") {
      printHeader("规则说明");
      std::cout << "近战 j 有距离限制；商店购买长弓后可 r 远程，无视距离。\n"
                << "伤害 = 攻击减防；弓箭为 65% 攻击并只计一半防御。暴击与狂战斧可叠加。\n"
                << "普通怪按数值约 4 刀近战、10 箭内可击杀（无暴击基准）。\n"
                << "战斗中按 s 可随时打开商店（不消耗回合）；关卡内战斗间隙也可按 s 进商店。\n"
                << "第 3、6、9 关第二场为精英怪。\n";
      pauseEnter();
      continue;
    }
    if (cmd != "1") continue;
    if (!runStage(g)) {
      printHeader("游戏结束");
      std::cout << "最终积分: " << g.score << " 评级: " << gradeFromScore(g.score) << "\n";
      pauseEnter();
      return;
    }
    ++g.stage;
    if (g.stage > 9) {
      printHeader("通关");
      printHeroPanel(g);
      std::cout << "\n最终评级: " << gradeFromScore(g.score) << " 积分: " << g.score << "\n";
      pauseEnter();
      return;
    }
    std::cout << "\n是否进入商店？[y/N]: ";
    std::string yn;
    if (std::getline(std::cin, yn) && !yn.empty() && (yn[0] == 'y' || yn[0] == 'Y')) shopLoop(g);
  }
}
int main() {
    RunGame();
    return 0;
}
