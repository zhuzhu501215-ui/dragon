#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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

void pauseGame() {
  std::cout << "\n[ 按 Enter 继续... ]";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string repeat(char c, int n) { return std::string(std::max(0, n), c); }

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

const Equipment kShopCatalog[] = {
    {"wood_sword", "[武器] 木剑", "攻击 +5", 5, 0, 0, 0.0, EquipSpecial::None, 40},
    {"iron_armor", "[护甲] 铁甲", "防御 +8", 0, 8, 0, 0.0, EquipSpecial::None, 55},
    {"ruby", "[宝石] 红宝石", "最大生命 +25", 0, 0, 25, 0.0, EquipSpecial::None, 45},
    {"clover", "[挂件] 四叶草", "幸运 +0.05", 0, 0, 0, 0.05, EquipSpecial::None, 50},
};

Equipment makeBerserkerAxe() {
  Equipment e;
  e.id = "berserker_axe";
  e.name = "[专属] 狂战士之斧";
  e.desc = "攻击前消耗20点生命，本次伤害翻倍。";
  e.special = EquipSpecial::BerserkerAxe;
  return e;
}

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

  void recomputeStats() {
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

  bool hasBerserkerAxe() const {
    for (const auto& it : items) {
      if (it.special == EquipSpecial::BerserkerAxe) return true;
    }
    return false;
  }
};

void printHeader(const std::string& t);

Character makeVaneHellion() {
  Character c;
  c.name = "范·地狱火";
  c.baseAtk = 50;
  c.baseDef = 20;
  c.baseMaxHp = 150;
  c.baseLuck = 0.30;
  c.skillName = "地狱契约";
  c.skillDesc = "升到2星后解锁专属装备狂战士之斧。";
  c.recomputeStats();
  c.hp = c.maxHp;
  return c;
}

Character makeHero(const std::string& name, int atk, int def, int hp, double luck,
                   const std::string& skillName, const std::string& skillDesc) {
  Character c;
  c.name = name;
  c.baseAtk = atk;
  c.baseDef = def;
  c.baseMaxHp = hp;
  c.baseLuck = luck;
  c.skillName = skillName;
  c.skillDesc = skillDesc;
  c.recomputeStats();
  c.hp = c.maxHp;
  return c;
}

std::vector<Character> makeHeroRoster() {
  return {
      makeVaneHellion(),
      makeHero("盖伦", 42, 32, 185, 0.12, "坚韧", "每场战斗开始时临时防御+8。"),
      makeHero("薇恩", 51, 17, 138, 0.30, "圣银弩箭", "暴击时额外造成5点真实伤害。"),
      makeHero("卡特琳娜", 58, 13, 126, 0.26, "死亡绽放", "生命低于50%时攻击+10。"),
      makeHero("莫甘娜", 44, 25, 175, 0.19, "灵魂虹吸", "每次胜利后回复12生命。"),
  };
}

Character chooseHero() {
  const std::vector<Character> heroes = makeHeroRoster();
  while (true) {
    printHeader("选择你的英雄（共5名）");
    for (size_t i = 0; i < heroes.size(); ++i) {
      const Character& h = heroes[i];
      std::cout << "  [" << (i + 1) << "] " << h.name << "\n"
                << "      攻击 " << h.baseAtk << "  防御 " << h.baseDef << "  生命 "
                << h.baseMaxHp << "  幸运 " << std::fixed << std::setprecision(2)
                << h.baseLuck << std::defaultfloat << "\n"
                << "      技能：" << h.skillName << " - " << h.skillDesc << "\n";
    }
    std::cout << "\n输入编号：";
    std::string line;
    if (!std::getline(std::cin, line)) return heroes.front();
    int id = 0;
    try { id = std::stoi(line); } catch (...) { id = 0; }
    if (id >= 1 && id <= static_cast<int>(heroes.size())) {
      Character selected = heroes[static_cast<size_t>(id - 1)];
      std::cout << "\n你选择了：" << selected.name << "\n";
      pauseGame();
      return selected;
    }
    std::cout << "输入无效，请输入 1-" << heroes.size() << ".\n";
    pauseGame();
  }
}

struct Monster {
  std::string name;
  int hp = 0;
  int maxHp = 0;
  int atk = 0;
  int def = 0;
  int gold = 0;
  bool elite = false;
};

Monster makeSlime(bool elite) {
  Monster m{"史莱姆", 80, 80, 18, 5, 12, elite};
  if (elite) m = {"精英·史莱姆王", 160, 160, 32, 12, 35, true};
  return m;
}
Monster makeGoblin(bool elite) {
  Monster m{"哥布林", 95, 95, 24, 8, 15, elite};
  if (elite) m = {"精英·哥布林队长", 190, 190, 40, 16, 42, true};
  return m;
}
Monster makeWolf(bool elite) {
  Monster m{"野狼", 70, 70, 30, 4, 14, elite};
  if (elite) m = {"精英·霜狼", 150, 150, 48, 10, 40, true};
  return m;
}
Monster makeGolem(bool elite) {
  Monster m{"石像怪", 120, 120, 20, 18, 18, elite};
  if (elite) m = {"精英·岩石领主", 240, 240, 36, 28, 48, true};
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

struct CombatResult {
  bool win = false;
  int goldEarned = 0;
  std::vector<std::string> log;
};

int calcDamage(int rawAtk, int def) {
  return std::max(1, rawAtk - def);
}

struct GameState {
  Character hero;
  int stage = 1;
  int gold = 80;
  int score = 0;
  std::vector<Blessing> blessings;
  bool normalCleared = false;
  bool hellCleared = false;
  double customBossMultiplier = 3.0;
};

Monster scaledMonster(Monster m, double factor) {
  auto s = [factor](int v) { return std::max(1, static_cast<int>(std::lround(v * factor))); };
  m.maxHp = s(m.maxHp);
  m.hp = m.maxHp;
  m.atk = s(m.atk);
  m.def = s(m.def);
  m.gold = s(m.gold);
  return m;
}

Monster makeHellBoss() {
  return {"深渊魔王", 320, 320, 58, 22, 160, true};
}

double askBossMultiplier(double current) {
  while (true) {
    printHeader("自定义最终Boss倍率");
    std::cout << "当前倍率：x" << std::fixed << std::setprecision(2) << current
              << std::defaultfloat << "\n";
    std::cout << "输入新倍率 [1.00 ~ 10.00]，回车保持不变：";
    std::string line;
    if (!std::getline(std::cin, line)) return current;
    if (line.empty()) return current;
    try {
      double v = std::stod(line);
      if (v >= 1.0 && v <= 10.0) return v;
    } catch (...) {}
    std::cout << "输入无效。\n";
    pauseGame();
  }
}

void ensureVaneTwoStarGear(GameState& g) {
  if (g.hero.stars < 2 || g.hero.name != "范·地狱火") return;
  for (const auto& it : g.hero.items) if (it.special == EquipSpecial::BerserkerAxe) return;
  g.hero.items.push_back(makeBerserkerAxe());
  g.hero.recomputeStats();
}

int heroAttackDamage(GameState& g, int monsterDef, std::string& note) {
  note.clear();
  int dmg = calcDamage(g.hero.atk, monsterDef);
  if (g.hero.name == "卡特琳娜" && g.hero.hp * 2 < g.hero.maxHp) dmg += 10;
  if (g.hero.hasBerserkerAxe() && g.hero.hp > 20) {
    g.hero.hp -= 20;
    dmg *= 2;
    note += "[狂战士之斧：-20生命，伤害x2] ";
  }
  bool crit = rand01() < g.hero.luck;
  if (crit) {
    dmg *= 2;
    note += "[暴击：伤害x2] ";
    if (g.hero.name == "薇恩") {
      dmg += 5;
      note += "[圣银弩箭+5] ";
    }
  }
  return dmg;
}

CombatResult runBattle(GameState& g, Monster m) {
  CombatResult r;
  r.log.push_back(std::string(">>> 遭遇 ") + (m.elite ? "[精英] " : "") + m.name + " <<<");

  if (g.hero.name == "盖伦") {
    int oldDef = g.hero.def;
    g.hero.def += 8;
    r.log.push_back("[盖伦被动] 本场防御+8。");
    (void)oldDef;
  }

  int round = 0;
  while (g.hero.hp > 0 && m.hp > 0) {
    ++round;
    std::ostringstream line;
    line << "-- 回合 " << round << " --";

    std::string note;
    int hdmg = heroAttackDamage(g, m.def, note);
    m.hp = std::max(0, m.hp - hdmg);
    line << "\n  你造成 " << hdmg << " damage";
    if (!note.empty()) line << "  " << note;
    int bar = m.maxHp > 0 ? (20 * m.hp / m.maxHp) : 0;
    line << "\n  [" << repeat('#', bar) << repeat('.', 20 - bar) << "] " << m.hp << "/" << m.maxHp;
    r.log.push_back(line.str());

    if (m.hp <= 0) {
      r.win = true;
      r.goldEarned = m.gold;
      g.gold += m.gold;
      g.score += 50 + (m.elite ? 80 : 0) + std::max(0, g.hero.hp / 2);
      r.log.push_back("[胜利] 金币 +" + std::to_string(m.gold));
      if (g.hero.name == "莫甘娜") {
        g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 12);
        r.log.push_back("[莫甘娜被动] 回复12生命。");
      }
      break;
    }

    int md = calcDamage(m.atk, g.hero.def);
    g.hero.hp -= md;
    r.log.push_back("  " + m.name + " 反击造成 " + std::to_string(md) + " 伤害。你的生命：" +
                    std::to_string(std::max(0, g.hero.hp)) + "/" + std::to_string(g.hero.maxHp));

    if (g.hero.hp <= 0) {
      r.win = false;
      r.log.push_back("[失败] 你倒下了...");
      break;
    }
  }
  return r;
}

char gradeFromScore(int s) {
  if (s >= 900) return 'S';
  if (s >= 650) return 'A';
  if (s >= 400) return 'B';
  if (s >= 200) return 'C';
  return 'D';
}

void printBar(const std::string& title, int cur, int max, char fill, int width = 24) {
  int filled = (max > 0) ? static_cast<int>(std::llround((1.0 * cur / max) * width)) : 0;
  filled = std::max(0, std::min(filled, width));
  std::cout << "  " << title << " [";
  for (int i = 0; i < width; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << cur << "/" << max << "\n";
}

void printHeader(const std::string& t) {
  std::cout << "\n" << repeat('=', 62) << "\n";
  std::cout << t << "\n";
  std::cout << repeat('=', 62) << "\n";
}

void printHeroPanel(const GameState& g) {
  std::cout << "\n[Hero] " << g.hero.name << "  星级：" << g.hero.stars << "\n";
  printBar("生命", g.hero.hp, g.hero.maxHp, '#');
  std::cout << "  攻击 " << g.hero.atk << "  防御 " << g.hero.def << "  幸运 "
            << std::fixed << std::setprecision(2) << g.hero.luck << std::defaultfloat << "\n";
  std::cout << "  技能：" << g.hero.skillName << " - " << g.hero.skillDesc << "\n";
  std::cout << "  装备：";
  if (g.hero.items.empty()) std::cout << "(无)\n";
  else {
    for (size_t i = 0; i < g.hero.items.size(); ++i) {
      std::cout << g.hero.items[i].name << (i + 1 < g.hero.items.size() ? " | " : "\n");
    }
  }
  if (!g.blessings.empty()) {
    std::cout << "  祝福：";
    for (size_t i = 0; i < g.blessings.size(); ++i) {
      std::cout << g.blessings[i].name << (i + 1 < g.blessings.size() ? ", " : "\n");
    }
  }
  std::cout << "  金币：" << g.gold << "  关卡：" << g.stage << "/9  Score: " << g.score << "\n";
}

void shopLoop(GameState& g) {
  while (true) {
    printHeader("商店");
    printHeroPanel(g);
    std::cout << "\n  [1] Buy equipment\n"
              << "  [2] 购买祝福\n"
              << "  [3] 花费60金币：范·地狱火升到2星\n"
              << "  [4] 花费25金币：回复40生命\n"
              << "  [0] 离开商店\n\n请选择：";

    std::string line;
    if (!std::getline(std::cin, line)) return;
    if (line == "0") break;

    if (line == "1") {
      std::cout << "\n装备列表：\n";
      for (size_t i = 0; i < sizeof(kShopCatalog) / sizeof(kShopCatalog[0]); ++i) {
        const auto& e = kShopCatalog[i];
        std::cout << "  [" << (i + 1) << "] " << e.name << " - " << e.desc << " - " << e.price << " gold\n";
      }
      std::cout << "输入编号（0取消）：";
      std::string pick;
      if (!std::getline(std::cin, pick)) return;
      int id = 0;
      try { id = std::stoi(pick); } catch (...) { id = 0; }
      if (id <= 0) continue;
      size_t idx = static_cast<size_t>(id - 1);
      if (idx >= sizeof(kShopCatalog) / sizeof(kShopCatalog[0])) continue;
      const Equipment& e = kShopCatalog[idx];
      if (g.gold < e.price) {
        std::cout << "金币不足。\n";
        pauseGame();
        continue;
      }
      g.gold -= e.price;
      g.hero.items.push_back(e);
      g.hero.recomputeStats();
      ensureVaneTwoStarGear(g);
      std::cout << "已购买：" << e.name << "\n";
      pauseGame();
    } else if (line == "2") {
      std::cout << "\n祝福列表：\n"
                << "  [1] 战神祝福    攻击+5   (55金币)\n"
                << "  [2] 守护祝福    防御+6   (55金币)\n"
                << "  [3] 生命祝福    生命+30   (60金币)\n"
                << "  [4] 幸运祝福    幸运+0.03 (65金币)\n"
                << "  [0] 取消\n请选择：";
      std::string pick;
      if (!std::getline(std::cin, pick)) return;
      Blessing b;
      int cost = 0;
      if (pick == "1") { b = {"战神", "攻击+5", 5, 0, 0, 0}; cost = 55; }
      else if (pick == "2") { b = {"守护", "防御+6", 0, 6, 0, 0}; cost = 55; }
      else if (pick == "3") { b = {"生命", "生命+30", 0, 0, 30, 0}; cost = 60; }
      else if (pick == "4") { b = {"幸运", "幸运+0.03", 0, 0, 0, 0.03}; cost = 65; }
      else continue;

      if (g.gold < cost) {
        std::cout << "金币不足。\n";
        pauseGame();
        continue;
      }
      g.gold -= cost;
      g.blessings.push_back(b);
      g.hero.baseAtk += b.atk;
      g.hero.baseDef += b.def;
      g.hero.baseMaxHp += b.hp;
      g.hero.baseLuck += b.luck;
      g.hero.recomputeStats();
      if (b.hp > 0) g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + b.hp);
      ensureVaneTwoStarGear(g);
      std::cout << "获得祝福：" << b.name << "\n";
      pauseGame();
    } else if (line == "3") {
      if (g.hero.name != "范·地狱火") {
        std::cout << "仅范·地狱火可在此升星。\n";
        pauseGame();
        continue;
      }
      if (g.hero.stars >= 2) {
        std::cout << "已经是2星或更高。\n";
        pauseGame();
        continue;
      }
      if (g.gold < 60) {
        std::cout << "金币不足。\n";
        pauseGame();
        continue;
      }
      g.gold -= 60;
      g.hero.stars = 2;
      ensureVaneTwoStarGear(g);
      std::cout << "升星成功！已解锁狂战士之斧。\n";
      pauseGame();
    } else if (line == "4") {
      if (g.gold < 25) {
        std::cout << "金币不足。\n";
        pauseGame();
        continue;
      }
      g.gold -= 25;
      g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 40);
      std::cout << "已回复40生命。\n";
      pauseGame();
    }
  }
}

bool runStage(GameState& g) {
  printHeader("第" + std::to_string(g.stage));
  printHeroPanel(g);

  bool hasElite = (g.stage % 3 == 0);
  std::cout << "\n本关共有3场战斗";
  if (hasElite) std::cout << "（第2场为精英）";
  std::cout << ".\n";
  pauseGame();

  for (int i = 1; i <= 3; ++i) {
    Monster m = (hasElite && i == 2) ? randomElite() : randomCommon();
    printHeader("战斗 " + std::to_string(i) + "/3");
    CombatResult cr = runBattle(g, m);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) return false;
    printHeroPanel(g);
    pauseGame();
  }

  g.score += 100;
  std::cout << "\n*** 第" << g.stage << " 关通关 ***\n";
  std::cout << "当前评级：" << gradeFromScore(g.score) << " (Score " << g.score << ")\n";
  return true;
}

bool runHellMode(GameState& g) {
  struct HellStage { std::string name; Monster m; double factor; };
  std::vector<HellStage> hs = {
      {"地狱精英关 I", randomElite(), 1.5},
      {"地狱精英关 II", randomElite(), 2.0},
      {"地狱Boss关", makeHellBoss(), g.customBossMultiplier},
  };

  printHeader("地狱模式");
  std::cout << "共3关：精英x1.5，精英x2.0，Boss x" << std::fixed << std::setprecision(2)
            << g.customBossMultiplier << std::defaultfloat << "\n";
  g.hero.hp = g.hero.maxHp;
  std::cout << "进入地狱模式前已回满生命。\n";
  pauseGame();

  for (size_t i = 0; i < hs.size(); ++i) {
    Monster mm = scaledMonster(hs[i].m, hs[i].factor);
    mm.name = hs[i].name + " - " + mm.name;

    printHeader("地狱战斗 " + std::to_string(i + 1) + "/3");
    std::cout << "敌人倍率：x" << std::fixed << std::setprecision(2)
              << hs[i].factor << std::defaultfloat << "\n";
    pauseGame();

    CombatResult cr = runBattle(g, mm);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) {
      printHeader("地狱模式 Failed");
      std::cout << "最终积分：" << g.score << "\n";
      pauseGame();
      return false;
    }
    g.score += 180 + static_cast<int>(i) * 80;
    printHeroPanel(g);
    pauseGame();
  }

  g.hellCleared = true;
  printHeader("地狱模式 Cleared");
  std::cout << "成就解锁：地狱征服者\n";
  std::cout << "功能解锁：可自定义最终Boss倍率。\n";
  pauseGame();
  return true;
}

void mainMenu() {
  printHeader("金铲铲风格·5英雄控制台版");
  std::cout << "  规则说明:\n"
            << "  - 普通模式共9关，每3关出现1次精英战。\n"
            << "  - 幸运值决定暴击率（暴击=双倍伤害）。\n"
            << "  - After clearing normal mode, 地狱模式 is available.\n"
            << "  - 地狱模式: 2 elite stages + final Boss stage.\n\n";
}

}  // namespace

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  mainMenu();
  GameState g;
  g.hero = chooseHero();

  std::cout << "按 Enter 开始冒险...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  while (g.stage <= 9) {
    printHeader("主菜单 - 第" + std::to_string(g.stage));
    printHeroPanel(g);
    std::cout << "\n  [1] 进入战斗\n"
              << "  [2] 打开商店\n"
              << "  [3] 查看规则\n"
              << "  [0] 退出游戏\n"
              << "\n请选择：";

    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;

    if (cmd == "0") return 0;
    if (cmd == "2") { shopLoop(g); continue; }
    if (cmd == "3") {
      printHeader("规则说明");
      std::cout << "  - 生命上限封顶为300。\n"
                << "  - 伤害公式：max(1, 攻击-防御)。\n"
                << "  - 范·地狱火2星后解锁狂战士之斧。\n"
                << "  - 普通通关后开启地狱模式，倍率1.5/2.0/Boss自定义。\n\n";
      pauseGame();
      continue;
    }
    if (cmd != "1") continue;

    if (!runStage(g)) {
      printHeader("游戏结束");
      std::cout << "最终积分：" << g.score << "  评级：" << gradeFromScore(g.score) << "\n";
      pauseGame();
      return 0;
    }

    ++g.stage;
    if (g.stage > 9) break;

    std::cout << "\n下一关前是否进入商店？[y/N]：";
    std::string yn;
    if (std::getline(std::cin, yn) && !yn.empty() && (yn[0] == 'y' || yn[0] == 'Y')) {
      shopLoop(g);
    }
  }

  g.normalCleared = true;
  printHeader("普通模式通关");
  printHeroPanel(g);
  std::cout << "\n最终评级：" << gradeFromScore(g.score) << "  积分：" << g.score << "\n";
  pauseGame();

  while (true) {
    printHeader("通关后菜单");
    std::cout << "  [1] 开启地狱模式\n";
    if (g.hellCleared) {
      std::cout << "  [2] 设置最终Boss倍率（当前 x" << std::fixed << std::setprecision(2)
                << g.customBossMultiplier << std::defaultfloat << ")\n";
    }
    std::cout << "  [0] 退出游戏\n\n请选择：";

    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;
    if (cmd == "0") break;
    if (cmd == "1") {
      runHellMode(g);
    } else if (cmd == "2" && g.hellCleared) {
      g.customBossMultiplier = askBossMultiplier(g.customBossMultiplier);
    }
  }

  return 0;
}



