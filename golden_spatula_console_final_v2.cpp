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

struct GameState;

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

void pause() {
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

  Equipment() = default;
  Equipment(std::string id_, std::string name_, std::string desc_, int atk_, int def_,
            int hp_, double luck_, EquipSpecial special_, int price_)
      : id(std::move(id_)),
        name(std::move(name_)),
        desc(std::move(desc_)),
        atk(atk_),
        def(def_),
        hp(hp_),
        luck(luck_),
        special(special_),
        price(price_) {}
};

const Equipment kShopCatalog[] = {
    {"wood_sword", "[剑] 木剑", "攻击力 +5", 5, 0, 0, 0.0, EquipSpecial::None, 40},
    {"iron_armor", "[甲] 铁甲", "防御力 +8", 0, 8, 0, 0.0, EquipSpecial::None, 55},
    {"ruby", "[宝] 红宝石", "最大生命 +25", 0, 0, 25, 0.0, EquipSpecial::None, 45},
    {"clover", "[幸] 四叶草", "幸运 +0.05", 0, 0, 0, 0.05, EquipSpecial::None, 50},
};

Equipment makeBerserkerAxe() {
  Equipment e;
  e.id = "berserker_axe";
  e.name = "[斧] 狂战士之斧";
  e.desc = "每次攻击消耗 20 HP，当次伤害翻倍（范·地狱火二星专属）";
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
void printStartScreen();
void printEndingScreen(bool win, const GameState& g);
void printTutorialIntro();
void printTutorialMainMenuHint(GameState& g);
void printTutorialShopHint(GameState& g);
void printTutorialStageHint(GameState& g, bool hasElite);
void printTutorialPostStageHint(GameState& g);

Character makeVaneHellion() {
  Character c;
  c.name = "范·地狱火 (Vane Hellion)";
  c.baseAtk = 50;
  c.baseDef = 20;
  c.baseMaxHp = 150;
  c.baseLuck = 0.30;
  c.skillName = "地狱契约";
  c.skillDesc = "升至二星后获得专属装备「狂战士之斧」。";
  c.stars = 1;
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
  c.stars = 1;
  c.recomputeStats();
  c.hp = c.maxHp;
  return c;
}

std::vector<Character> makeHeroRoster() {
  std::vector<Character> heroes;
  heroes.reserve(5);
  heroes.push_back(makeVaneHellion());
  heroes.push_back(makeHero("盖伦 (Garen)", 42, 32, 185, 0.12, "坚韧旋击",
                            "每场战斗开始时获得 8 点临时防御。"));
  heroes.push_back(makeHero("薇恩 (Vayne)", 51, 17, 138, 0.30, "圣银弩箭",
                            "暴击时额外造成 5 点真实伤害。"));
  heroes.push_back(makeHero("卡特琳娜 (Katarina)", 58, 13, 126, 0.26, "死亡莲华",
                            "血量低于 50% 时攻击 +10。"));
  heroes.push_back(makeHero("莫甘娜 (Morgana)", 44, 25, 175, 0.19, "痛苦腐蚀",
                            "战斗胜利后回复 12 HP。"));
  return heroes;
}

Character chooseHero() {
  const std::vector<Character> heroes = makeHeroRoster();
  while (true) {
    // 角色选择页：展示基础面板，方便开局做流派决策（爆发/坦克/暴击等）。
    printHeader("选择你的角色（共 5 名）");
    for (size_t i = 0; i < heroes.size(); ++i) {
      const Character& h = heroes[i];
      std::cout << "  [" << (i + 1) << "] " << h.name << "\n"
                << "      ATK " << h.baseAtk << "  DEF " << h.baseDef << "  HP "
                << h.baseMaxHp << "  LUCK " << std::fixed << std::setprecision(2)
                << h.baseLuck << std::defaultfloat << "\n"
                << "      技能: " << h.skillName << " - " << h.skillDesc << "\n";
    }
    std::cout << "\n输入编号选择角色: ";
    std::string line;
    if (!std::getline(std::cin, line)) return heroes.front();
    int id = 0;
    try {
      id = std::stoi(line);
    } catch (...) {
      id = 0;
    }
    if (id >= 1 && id <= static_cast<int>(heroes.size())) {
      Character selected = heroes[static_cast<size_t>(id - 1)];
      std::cout << "\n你选择了: " << selected.name << "\n";
      pause();
      return selected;
    }
    // 输入兜底：防止非法输入导致直接退出选择流程。
    std::cout << "输入无效，请输入 1-" << heroes.size() << "。\n";
    pause();
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
  // [Boss] 0/1 表示普通或精英；boss 支持双命和阶段强化。
  bool boss = false;
  int lives = 1;
  int phase = 1;
};

Monster makeSlime(bool elite) {
  Monster m{"史莱姆", 80, 80, 18, 5, 12, elite};
  if (elite) {
    m.hp = m.maxHp = 160;
    m.atk = 32;
    m.def = 12;
    m.gold = 35;
    m.name = "精英·史莱姆王";
  }
  return m;
}

Monster makeGoblin(bool elite) {
  Monster m{"哥布林", 95, 95, 24, 8, 15, elite};
  if (elite) {
    m.hp = m.maxHp = 190;
    m.atk = 40;
    m.def = 16;
    m.gold = 42;
    m.name = "精英·哥布林酋长";
  }
  return m;
}

Monster makeWolf(bool elite) {
  Monster m{"野狼", 70, 70, 30, 4, 14, elite};
  if (elite) {
    m.hp = m.maxHp = 150;
    m.atk = 48;
    m.def = 10;
    m.gold = 40;
    m.name = "精英·霜狼";
  }
  return m;
}

Monster makeGolem(bool elite) {
  Monster m{"石像怪", 120, 120, 20, 18, 18, elite};
  if (elite) {
    m.hp = m.maxHp = 240;
    m.atk = 36;
    m.def = 28;
    m.gold = 48;
    m.name = "精英·岩石领主";
  }
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

Monster makeStageBoss(int stage) {
  Monster b = randomElite();
  b.boss = true;
  b.lives = 2;
  b.phase = 1;
  b.name = "Boss·" + b.name;
  b.gold += 20 + stage * 2;
  return b;
}

struct CombatResult {
  bool win = false;
  int goldEarned = 0;
  std::vector<std::string> log;
};

int calcDamage(int rawAtk, int def) {
  int d = rawAtk - def;
  return std::max(1, d);
}

struct GameState {
  Character hero;
  int stage = 1;
  int gold = 80;
  int score = 0;
  std::vector<Blessing> blessings;
  bool tutorialMode = false;
  bool tutorialMainMenuHintShown = false;
  bool tutorialShopHintShown = false;
  bool tutorialStageHintShown = false;
  bool tutorialBossHintShown = false;
  bool tutorialPostStageHintShown = false;
};

// [Curve] 难度曲线：按关卡提升怪物属性，前中后期增幅逐步提高。
int stageHpBonus(int stage) {
  // 前 3 关小幅提升，4-6 关中速提升，7+ 关高压提升。
  if (stage <= 1) return 0;
  if (stage <= 3) return 14 * (stage - 1);
  if (stage <= 6) return 32 + 20 * (stage - 3);
  return 92 + 26 * (stage - 6);
}

// [Curve] 难度曲线：攻击成长略低于血量，避免后期秒杀感太强。
int stageAtkBonus(int stage) {
  // 攻击成长稍缓于血量成长，避免后期被怪物瞬间打崩。
  if (stage <= 1) return 0;
  if (stage <= 3) return 3 * (stage - 1);
  if (stage <= 6) return 6 + 4 * (stage - 3);
  return 18 + 5 * (stage - 6);
}

// [Curve] 难度曲线：防御缓慢增长，保证输出构筑仍有价值。
int stageDefBonus(int stage) {
  // 防御曲线压得更平，让高攻击构筑始终有存在感。
  if (stage <= 2) return 0;
  if (stage <= 6) return stage - 2;
  return 4 + (stage - 6) * 2;
}

// [Curve] 掉落曲线：关卡越高，金币倍率越高；精英额外加成。
int goldDropMultiplierPercent(int stage, bool elite) {
  // 关卡越高金币倍率越高；精英/Boss 额外奖励用于支撑商店消费。
  int p = 100 + (stage - 1) * 8;
  if (elite) p += 18;
  return p;
}

// [Curve] 通关奖励曲线：用于每关结算时的保底金币补给。
int stageClearGoldBonus(int stage) {
  // 每关通关保底金币，降低“卡关后没钱补强”的挫败感。
  if (stage <= 3) return 8 + stage * 2;
  if (stage <= 6) return 16 + stage * 3;
  return 26 + stage * 4;
}

// [Curve] 将基础怪物按当前关卡缩放，形成平滑难度爬升。
Monster scaleMonsterForStage(Monster m, int stage) {
  // 统一缩放入口：所有怪物进入战斗前都走这套成长规则。
  m.maxHp += stageHpBonus(stage) + (m.elite ? stage * 6 : 0);
  m.hp = m.maxHp;
  m.atk += stageAtkBonus(stage) + (m.elite ? stage / 2 : 0);
  m.def += stageDefBonus(stage);
  m.gold = std::max(1, (m.gold * goldDropMultiplierPercent(stage, m.elite)) / 100);
  // [Boss] Boss 作为关卡核心战，基础面板再额外强化一档。
  if (m.boss) {
    // Boss 额外乘区：突出关卡节点战的压迫感和奖励感。
    m.maxHp = m.maxHp * 11 / 10;
    m.hp = m.maxHp;
    m.atk += 4 + stage;
    m.def += 2;
    m.gold = m.gold * 13 / 10;
  }
  return m;
}

void ensureVaneTwoStarGear(GameState& g) {
  if (g.hero.stars < 2) return;
  for (const auto& it : g.hero.items) {
    if (it.special == EquipSpecial::BerserkerAxe) return;
  }
  g.hero.items.push_back(makeBerserkerAxe());
  g.hero.recomputeStats();
  g.hero.hp = std::min(g.hero.hp, g.hero.maxHp);
}

int heroAttackDamage(GameState& g, int monsterDef, bool& crit, bool& axeUsed,
                     std::string& note) {
  crit = false;
  axeUsed = false;
  note.clear();
  int base = calcDamage(g.hero.atk, monsterDef);

  bool hasAxe = g.hero.hasBerserkerAxe();
  if (hasAxe && g.hero.hp > 20) {
    // 专属斧子是“以血换爆发”的风险收益机制。
    g.hero.hp -= 20;
    base *= 2;
    axeUsed = true;
    note += "[狂战士之斧] -20 HP，伤害翻倍 ";
  }

  if (rand01() < g.hero.luck) {
    // 幸运直接对应暴击概率，和斧子倍率可叠加。
    base *= 2;
    crit = true;
    note += "[幸运暴击] 双倍伤害 ";
  }
  return base;
}

int monsterAttack(GameState& g, Monster& m, std::string& note) {
  note.clear();
  int dmg = calcDamage(m.atk, g.hero.def);
  // [Boss] 二阶段技能：狂怒重击（高额追加伤害）+ 灼烧（固定掉血）。
  if (m.boss && m.phase >= 2) {
    // 二阶段技能 1：在普通攻击上追加一次重击。
    if (rand01() < 0.38) {
      int extra = std::max(3, m.atk / 3);
      dmg += extra;
      note += "[Boss二阶段·狂怒重击] +" + std::to_string(extra) + " ";
    }
    // 二阶段技能 2：独立判定的持续性压血效果。
    if (rand01() < 0.28) {
      g.hero.hp -= 6;
      note += "[Boss二阶段·灼烧] 额外-6HP ";
    }
  }
  g.hero.hp -= dmg;
  return dmg;
}

CombatResult runBattle(GameState& g, Monster m) {
  CombatResult r;
  // [Curve] 在战斗入口按关卡缩放怪物属性和掉落。
  m = scaleMonsterForStage(m, g.stage);
  std::ostringstream header;
  header << ">>> 遭遇 " << (m.boss ? "[Boss] " : (m.elite ? "[精英] " : "")) << m.name << " <<<";
  r.log.push_back(header.str());

  int round = 0;
  while (g.hero.hp > 0 && m.hp > 0) {
    // 回合结构：先手（玩家）-> 结算击杀 -> 反击（怪物）-> 生存检查。
    ++round;
    std::ostringstream line;
    line << "-- 第 " << round << " 回合 --";

    bool crit = false, axe = false;
    std::string note;
    int hdmg = heroAttackDamage(g, m.def, crit, axe, note);
    m.hp -= hdmg;
    line << "\n  [#] 你 造成 " << hdmg << " 伤害";
    if (!note.empty()) line << "  " << note;
    if (m.hp < 0) m.hp = 0;
    int bar = m.maxHp > 0 ? (20 * m.hp / m.maxHp) : 0;
    bar = std::max(0, std::min(bar, 20));
    line << "\n  [" << repeat('#', bar) << repeat('.', 20 - bar) << "] "
         << m.hp << "/" << m.maxHp;

    r.log.push_back(line.str());

    if (m.hp <= 0) {
      // [Boss] 第一条命打空后进入二阶段（第二条命）。
      if (m.boss && m.lives > 1) {
        --m.lives;
        m.phase = 2;
        m.maxHp = std::max(1, m.maxHp * 70 / 100);
        m.hp = m.maxHp;
        m.atk += 8 + g.stage * 2;
        m.def += 3 + g.stage / 2;
        // 进入二阶段后立即继续战斗，不发放击杀奖励。
        std::ostringstream phaseShift;
        phaseShift << "  [!!] " << m.name
                   << " 进入二阶段：第二条命激活，攻击与防御大幅提升！";
        r.log.push_back(phaseShift.str());
        continue;
      }
      r.win = true;
      r.goldEarned = m.gold;
      std::ostringstream win;
      win << "\n  [+] 击败！获得金币 +" << m.gold;
      r.log.push_back(win.str());
      g.gold += m.gold;
      g.score += 50 + (m.elite ? 80 : 0) + std::max(0, g.hero.hp / 2);
      break;
    }

    std::string mnote;
    int md = monsterAttack(g, m, mnote);
    std::ostringstream defl;
    defl << "  [*] " << m.name << " 反击 " << md << " → 你剩余 HP "
         << std::max(0, g.hero.hp) << "/" << g.hero.maxHp;
    if (!mnote.empty()) defl << "  " << mnote;
    r.log.push_back(defl.str());

    if (g.hero.hp <= 0) {
      r.win = false;
      r.log.push_back("\n  [X] 你倒下了...");
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
  int filled = 0;
  if (max > 0) filled = static_cast<int>(std::llround((1.0 * cur / max) * width));
  filled = std::max(0, std::min(filled, width));
  std::cout << "  " << title << " [";
  for (int i = 0; i < width; ++i) std::cout << (i < filled ? fill : '.');
  std::cout << "] " << cur << "/" << max << "\n";
}

void printHeader(const std::string& t) {
  std::cout << "\n"
            << "╔" << repeat('=', 58) << "╗\n"
            << "║ " << std::left << std::setw(56) << t << " ║\n"
            << "╚" << repeat('=', 58) << "╝\n";
}

void printHeroPanel(const GameState& g) {
  std::cout << "\n┌── 角色 ────────────────────────────────────────────────┐\n";
  std::cout << "│ " << g.hero.name << "  ★" << g.hero.stars << "\n";
  printBar("HP ", g.hero.hp, g.hero.maxHp, '#');
  std::cout << "│ ATK " << g.hero.atk << "   DEF " << g.hero.def
            << "   LUCK " << std::fixed << std::setprecision(2) << g.hero.luck << std::defaultfloat
            << "\n";
  std::cout << "│ 技能: " << g.hero.skillName << " — " << g.hero.skillDesc << "\n";
  std::cout << "│ 装备: ";
  if (g.hero.items.empty()) {
    std::cout << "(无)\n";
  } else {
    for (size_t i = 0; i < g.hero.items.size(); ++i) {
      std::cout << g.hero.items[i].name;
      if (i + 1 < g.hero.items.size()) std::cout << " | ";
    }
    std::cout << "\n";
  }
  if (!g.blessings.empty()) {
    std::cout << "│ 赐福: ";
    for (size_t i = 0; i < g.blessings.size(); ++i) {
      std::cout << g.blessings[i].name;
      if (i + 1 < g.blessings.size()) std::cout << ", ";
    }
    std::cout << "\n";
  }
  std::cout << "│ 金币: " << g.gold << "    关卡: " << g.stage << "/9"
            << "    积分: " << g.score << "\n";
  std::cout << "└────────────────────────────────────────────────────────┘\n";
}

void shopLoop(GameState& g) {
  while (true) {
    printTutorialShopHint(g);
    printHeader("商 城");
    printHeroPanel(g);
    std::cout
        << "\n  [1] 购买装备\n"
        << "  [2] 购买赐福\n"
        << "  [3] 花费 60 金币：范·地狱火 升星 (→2星，解锁狂战士之斧)\n"
        << "  [4] 花费 25 金币：回复 40 HP（不超过上限）\n"
        << "  [0] 离开商店\n"
        << "\n请选择: ";
    std::string line;
    if (!std::getline(std::cin, line)) return;
    if (line == "0") break;
    if (line == "1") {
      std::cout << "\n可购装备：\n";
      for (size_t i = 0; i < sizeof(kShopCatalog) / sizeof(kShopCatalog[0]); ++i) {
        const auto& e = kShopCatalog[i];
        std::cout << "  [" << (i + 1) << "] " << e.name << " — " << e.desc << " — 价格 " << e.price
                  << "\n";
      }
      std::cout << "输入编号购买，0 返回: ";
      std::string pick;
      if (!std::getline(std::cin, pick)) return;
      int id = 0;
      try {
        id = std::stoi(pick);
      } catch (...) {
        id = 0;
      }
      if (id <= 0) continue;
      size_t idx = static_cast<size_t>(id - 1);
      if (idx >= sizeof(kShopCatalog) / sizeof(kShopCatalog[0])) continue;
      const Equipment& e = kShopCatalog[idx];
      if (g.gold < e.price) {
        std::cout << "金币不足。\n";
        pause();
        continue;
      }
      g.gold -= e.price;
      g.hero.items.push_back(e);
      g.hero.recomputeStats();
      g.hero.hp = std::min(g.hero.hp, g.hero.maxHp);
      ensureVaneTwoStarGear(g);
      std::cout << "已购买 " << e.name << "\n";
      pause();
    } else if (line == "2") {
      std::cout << "\n赐福列表：\n"
                << "  [1] 战神祝福 — ATK+5 — 55 金币\n"
                << "  [2] 守护祝福 — DEF+6 — 55 金币\n"
                << "  [3] 生命祝福 — 最大 HP+30 — 60 金币\n"
                << "  [4] 幸运祝福 — LUCK+0.03 — 65 金币\n"
                << "0 返回\n选: ";
      std::string pick;
      if (!std::getline(std::cin, pick)) return;
      Blessing b;
      int cost = 0;
      if (pick == "1") {
        b = {"战神祝福", "ATK+5", 5, 0, 0, 0};
        cost = 55;
      } else if (pick == "2") {
        b = {"守护祝福", "DEF+6", 0, 6, 0, 0};
        cost = 55;
      } else if (pick == "3") {
        b = {"生命祝福", "最大HP+30", 0, 0, 30, 0};
        cost = 60;
      } else if (pick == "4") {
        b = {"幸运祝福", "LUCK+0.03", 0, 0, 0, 0.03};
        cost = 65;
      } else {
        continue;
      }
      if (g.gold < cost) {
        std::cout << "金币不足。\n";
        pause();
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
      std::cout << "获得赐福：" << b.name << "\n";
      pause();
    } else if (line == "3") {
      if (g.hero.name.find("范·地狱火") == std::string::npos) {
        std::cout << "仅范·地狱火可升星。\n";
        pause();
        continue;
      }
      if (g.hero.stars >= 2) {
        std::cout << "已是二星或以上。\n";
        pause();
        continue;
      }
      if (g.gold < 60) {
        std::cout << "金币不足。\n";
        pause();
        continue;
      }
      g.gold -= 60;
      g.hero.stars = 2;
      ensureVaneTwoStarGear(g);
      std::cout << "升星成功！获得专属装备「狂战士之斧」。\n";
      pause();
    } else if (line == "4") {
      if (g.gold < 25) {
        std::cout << "金币不足。\n";
        pause();
        continue;
      }
      g.gold -= 25;
      g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 40);
      std::cout << "已回复 HP。\n";
      pause();
    }
  }
}

bool runStage(GameState& g) {
  printHeader("第 " + std::to_string(g.stage) + " 关");
  printHeroPanel(g);

  bool hasElite = (g.stage % 3 == 0);
  printTutorialStageHint(g, hasElite);
  std::cout << "\n本关将连续遭遇 3 只怪兽";
  if (hasElite) std::cout << "（第 2 只为精英）";
  std::cout << "。\n";
  pause();

  for (int i = 1; i <= 3; ++i) {
    Monster m;
    if (hasElite && i == 2)
      // 每 3 关的中间战固定 Boss，用于形成节奏峰值。
      m = makeStageBoss(g.stage);
    else
      m = randomCommon();

    printHeader("战斗 " + std::to_string(i) + "/3");
    CombatResult cr = runBattle(g, m);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) return false;
    printHeroPanel(g);
    pause();
  }

  std::cout << "\n*** 第 " << g.stage << " 关 通关！***\n";
  // [Curve] 关卡金币保底奖励，缓解中后期商店压力。
  int clearGold = stageClearGoldBonus(g.stage);
  g.gold += clearGold;
  std::cout << "[关卡奖励] 金币 +" << clearGold << "\n";

  // [Curve] 分数奖励随关卡提升，强调后期挑战价值。
  int stageScore = 90 + g.stage * 20;
  g.score += stageScore;
  std::cout << "[关卡奖励] 积分 +" << stageScore << "\n";
  char gr = gradeFromScore(g.score);
  std::cout << "当前评级参考: " << gr << " 级（积分 " << g.score << "）\n";
  printTutorialPostStageHint(g);
  return true;
}

void mainMenu() {
  printHeader("金铲铲风格 · 控制台试玩版");
  std::cout << "  操作说明：根据提示输入数字或命令即可。\n"
            << "  关卡共 9 关；每 3 关会出现一只精英怪（本关第 2 战）。\n"
            << "  怪兽被击败会掉落金币；角色「幸运」为暴击（双倍伤害）概率。\n\n";
}

void printStartScreen() {
  std::cout << "\n";
  std::cout << "==============================================================\n";
  std::cout << "   *****   *****  *      *****   *****  *   *  *      ***** \n";
  std::cout << "  *       *   *  *      *   *  *      **  *  *      *      \n";
  std::cout << "  *  ***  *   *  *      *   *  ****   * * *  *      ****   \n";
  std::cout << "  *    *  *   *  *      *   *  *      *  **  *      *      \n";
  std::cout << "   *****   *****  *****  *****  *****  *   *  *****  ***** \n";
  std::cout << "---------------------------------------------------------------\n";
  std::cout << "                 TACTIC BATTLE - CONSOLE EDITION             \n";
  std::cout << "---------------------------------------------------------------\n";
  std::cout << "  # Build your hero (@_@)  # Beat stage bosses <V::V>  # Reach rank S [*]\n";
  std::cout << "================================================================\n";
  std::cout << "\n";
}

void printEndingScreen(bool win, const GameState& g) {
  std::cout << "
";
std::cout << "*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==\n";
std::cout << " **     **   ******   **    **          **         **  ****   **    **\n";
std::cout << "  **   **   **    **  **    **          **   **    **   **    ***   **\n";
std::cout << "   ** **    **    **  **    **           ** ** ** **    **    ****  **\n";
std::cout << "    ***     **    **  **    **            ***   ***     **    ** ** **\n";
std::cout << "    ***     **    **  **    **             **   **      **    **  ****\n";
std::cout << "    ***     **    **  **    **             **   **      **    **   ***\n";
std::cout << "    ***      ******    ******              **   **     ****   **    **\n";
std::cout << "*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==\n";
std::cout << "                             CHAMPION CLEAR                        \n";
std::cout << "...................................***..................................\n";
  } else {
    std::cout << "  *******     ******      **      **   *******          *******   **      **  ********  *******
";
    std::cout << " **         **      **   ***    ***   **              **     **   **    **   **        **    **
";
    std::cout << " **   ****  **      **   ** *  * **   **   ****       **     **    **  **    **        **    **
";
    std::cout << " **     **  **********   **  **  **   **     **       **     **     ****     ******    ******
";
    std::cout << " **     **  **      **   **      **   **     **       **     **     ****     **        **  **
";
    std::cout << " **     **  **      **   **      **   **     **       **     **    **  **    **        **   **
";
    std::cout << "  *******   **      **   **      **    *******         *******    **    **   ********  **    **
";
    std::cout << "*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*
";
    std::cout << "                        GAME OVER                             
";
  }
  std::cout << ".........................***.................................
";
  std::cout << "  Hero : " << g.hero.name << "
";
  std::cout << "  Score: " << g.score << "   Rank: " << gradeFromScore(g.score) << "
";
  std::cout << "  Stage: " << std::min(9, g.stage) << "/9   Gold: " << g.gold << "
";
  std::cout << "*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*
";
  std::cout << "
";
}

void printTutorialIntro() {
  printHeader("新手教程已开启");
  std::cout
      << "  你已进入引导模式，关键节点会自动提示一次。\n"
      << "  推荐节奏：先保命（HP/DEF）→ 再补输出（ATK/LUCK）。\n"
      << "  第 3/6/9 关第 2 战是双命 Boss，进关前优先补血。\n";
  pause();
}

void printTutorialMainMenuHint(GameState& g) {
  if (!g.tutorialMode || g.tutorialMainMenuHintShown) return;
  printHeader("教程提示：主菜单");
  std::cout
      << "  [1] 进入战斗：推进关卡，是主要流程。\n"
      << "  [2] 打开商城：补属性和回复 HP，打 Boss 前很关键。\n"
      << "  [3] 规则说明：查看机制细节。\n"
      << "  建议新手先逛一次商店再开打。\n";
  g.tutorialMainMenuHintShown = true;
  pause();
}

void printTutorialShopHint(GameState& g) {
  if (!g.tutorialMode || g.tutorialShopHintShown) return;
  printHeader("教程提示：商城");
  std::cout
      << "  新手优先级：\n"
      << "  1) 保命：铁甲 / 生命祝福 / 回复 HP\n"
      << "  2) 输出：木剑 / 战神祝福\n"
      << "  3) 暴击：四叶草 / 幸运祝福\n"
      << "  小技巧：别把金币全省着，当前战力更重要。\n";
  g.tutorialShopHintShown = true;
  pause();
}

void printTutorialStageHint(GameState& g, bool hasElite) {
  if (!g.tutorialMode) return;
  if (!g.tutorialStageHintShown) {
    printHeader("教程提示：战斗前");
    std::cout
        << "  每关共 3 场战斗，你不能在关内进商店。\n"
        << "  所以开打前要确认血量和装备是否到位。\n"
        << "  伤害公式：max(1, ATK - DEF)，暴击会双倍。\n";
    g.tutorialStageHintShown = true;
    pause();
  }
  if (hasElite && !g.tutorialBossHintShown) {
    printHeader("教程提示：Boss 关");
    std::cout
        << "  本关第 2 战是双命 Boss。\n"
        << "  Boss 第一条命打空后会进入二阶段并变强。\n"
        << "  建议：保持高 HP，必要时先在商店买回复再进关。\n";
    g.tutorialBossHintShown = true;
    pause();
  }
}

void printTutorialPostStageHint(GameState& g) {
  if (!g.tutorialMode || g.tutorialPostStageHintShown) return;
  printHeader("教程提示：通关结算");
  std::cout
      << "  通关后会获得关卡保底金币 + 积分。\n"
      << "  下一关前可再次进商店，优先补短板属性。\n"
      << "  若经常倒在 Boss，先提高 HP/DEF 再补伤害。\n";
  g.tutorialPostStageHintShown = true;
  pause();
}

}  // namespace

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  printStartScreen();
  mainMenu();
  GameState g;
  printHeader("教程模式");
  std::cout << "是否开启新手引导？[Y/n]: ";
  std::string tutorialChoice;
  if (std::getline(std::cin, tutorialChoice)) {
    g.tutorialMode =
        tutorialChoice.empty() || tutorialChoice[0] == 'y' || tutorialChoice[0] == 'Y';
  }
  if (g.tutorialMode) printTutorialIntro();
  g.hero = chooseHero();

  std::cout << "按 Enter 开始冒险...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  while (g.stage <= 9) {
    printTutorialMainMenuHint(g);
    printHeader("主菜单 — 第 " + std::to_string(g.stage) + " 关前");
    printHeroPanel(g);
    std::cout << "\n  [1] 进入战斗（推进关卡）\n"
              << "  [2] 打开商城\n"
              << "  [3] 查看规则说明\n"
              << "  [0] 退出游戏\n"
              << "\n请选择: ";

    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;

    if (cmd == "0") {
      std::cout << "再见！\n";
      return 0;
    }
    if (cmd == "3") {
      printHeader("规则说明");
      std::cout
          << "  · 属性：HP∈[0,300]，攻击/防御非负，幸运为暴击概率。\n"
          << "  · 范·地狱火二星后装备「狂战士之斧」：攻击前若 HP>20，扣 20 HP 并使当次伤害翻倍。\n"
          << "  · 暴击与斧子翻倍可叠加（先斧后判定幸运）。\n"
          << "  · 每关 3 场战斗；第 3、6、9 关第 2 场为双命 Boss，第二阶段技能更强。\n"
          << "  · 难度曲线：怪物属性会随关卡成长；掉落和关卡奖励也会同步提升。\n\n";
      pause();
      continue;
    }
    if (cmd == "2") {
      shopLoop(g);
      continue;
    }
    if (cmd != "1") continue;

    if (!runStage(g)) {
      printHeader("游戏结束");
      std::cout << "最终积分: " << g.score << "  评级: " << gradeFromScore(g.score) << "\n";
      printEndingScreen(false, g);
      pause();
      return 0;
    }

    ++g.stage;
    if (g.stage > 9) {
      printHeader("恭喜通关！");
      printHeroPanel(g);
      std::cout << "\n最终评级: " << gradeFromScore(g.score) << "  积分: " << g.score << "\n";
      printEndingScreen(true, g);
      pause();
      return 0;
    }

    std::cout << "\n进入下一关前可访问商店。\n是否打开商城？[y/N]: ";
    std::string yn;
    if (std::getline(std::cin, yn) && !yn.empty() && (yn[0] == 'y' || yn[0] == 'Y')) {
      shopLoop(g);
    }
  }

  return 0;
}
