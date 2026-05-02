#include "hero_system.h"

namespace Game {

Equipment makeBerserkerAxe() {
  Equipment e;
  e.id = "berserker_axe";
  e.name = "[Axe] Berserker Axe";
  e.desc = "Spend 20 HP per attack; that attack deals double damage (Vane Hellion 2-star exclusive).";
  e.special = EquipSpecial::BerserkerAxe;
  return e;
}

Character makeVaneHellion() {
  Character c;
  c.name = "Vane Hellion";
  c.baseAtk = 50;
  c.baseDef = 20;
  c.baseMaxHp = 150;
  c.baseLuck = 0.30;
  c.skillName = "Hell Pact";
  c.skillDesc = "Gain exclusive equipment \"Berserker Axe\" after reaching 2 stars.";
  c.stars = 1;
  c.recomputeStats();
  c.hp = c.maxHp;
  c.stamina = c.maxStamina;
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
  c.stamina = c.maxStamina;
  return c;
}

std::vector<Character> makeHeroRoster() {
  std::vector<Character> heroes;
  heroes.reserve(5);
  heroes.push_back(makeVaneHellion());
  heroes.push_back(makeHero("Garen", 42, 32, 185, 0.12, "Fortitude Spin",
                            "Gain 8 temporary defense at the start of each battle."));
  heroes.push_back(makeHero("Vayne", 51, 17, 138, 0.30, "Silver Bolts",
                            "Critical hits deal 5 bonus true damage."));
  heroes.push_back(makeHero("Katarina", 58, 13, 126, 0.26, "Death Lotus",
                            "Gain +10 ATK when HP is below 50%."));
  heroes.push_back(makeHero("Morgana", 44, 25, 175, 0.19, "Torment Corrosion",
                            "Recover 12 HP after winning a battle."));
  return heroes;
}

std::vector<std::string> heroModelFrame(const std::string& heroName, int frame) {
  int f = ((frame % 3) + 3) % 3;
  if (heroName.find("Vane Hellion") != std::string::npos) {
    if (f == 0)
      return {"    /\\    ", " __/OO\\__ ", "   /||\\   ", "   /  \\   ", " _/____\\_ "};
    if (f == 1)
      return {"    /\\    ", " __-OO\\__ ", "   /||\\   ", "  /   \\   ", " _/____\\_ "};
    return {"    /\\    ", " __/OO-\\_ ", "   /||\\   ", "   /  \\   ", " _/____\\_ "};
  }
  if (heroName.find("Garen") != std::string::npos) {
    if (f == 0)
      return {"  .---.   ", " (o_o )   ", " /|###\\   ", " /|###|\\  ", "  /___\\   "};
    if (f == 1)
      return {"  .---.   ", " (o_o )   ", " /|###|\\  ", " /|###\\   ", "  /___\\   "};
    return {"  .---.   ", " (-.- )   ", " /|###\\   ", " /|###|\\  ", "  /___\\   "};
  }
  if (heroName.find("Vayne") != std::string::npos) {
    if (f == 0)
      return {"  /\\_/\\   ", " (o_o )==>", " /|_ _|\\  ", "  / | \\   ", " _/_ _\\_  "};
    if (f == 1)
      return {"  /\\_/\\   ", " (o_o )=> ", " /|_ _|\\  ", "  / | \\   ", " _/_ _\\_  "};
    return {"  /\\_/\\   ", " (o_o )==>", " /|_ _| \\ ", "  / | \\   ", " _/_ _\\_  "};
  }
  if (heroName.find("Katarina") != std::string::npos) {
    if (f == 0)
      return {"   /\\     ", " _(x_x)_  ", " <=| |=<  ", "  / | \\   ", " _/_ _\\_  "};
    if (f == 1)
      return {"   /\\     ", " _(x_x)_  ", "  <| |=<  ", "  / | \\   ", " _/_ _\\_  "};
    return {"   /\\     ", " _(x_x)_  ", "<=| |= > ", "  / | \\   ", " _/_ _\\_  "};
  }
  if (heroName.find("Morgana") != std::string::npos) {
    if (f == 0)
      return {" /\\___/\\  ", " ( -.- )  ", " /|\\_/|\\  ", "  / | \\   ", " _/___\\_  "};
    if (f == 1)
      return {" /\\___/\\  ", " ( -.- )  ", " /| \\_|\\  ", "  / | \\   ", " _/___\\_  "};
    return {" /\\___/\\  ", " ( -.- )  ", " /|\\_/| \\ ", "  / | \\   ", " _/___\\_  "};
  }
  if (f == 0)
    return {"  .---.   ", " (o_o )   ", "  /|\\     ", "  / \\     ", " _/___\\_  "};
  if (f == 1)
    return {"  .---.   ", " (o_o )   ", "   /|\\    ", "  / \\     ", " _/___\\_  "};
  return {"  .---.   ", " (-.- )   ", "  /|\\     ", "  / \\     ", " _/___\\_  "};
}

std::vector<std::string> heroModel(const std::string& heroName) {
  return heroModelFrame(heroName, 0);
}

Character chooseHero() {
  const std::vector<Character> heroes = makeHeroRoster();
  while (true) {
    std::cout << "\n"
              << "╔" << repeat('=', 58) << "╗\n"
              << "║ " << std::left << std::setw(56) << "Choose Your Hero (5 total)" << " ║\n"
              << "╚" << repeat('=', 58) << "╝\n";
    for (size_t i = 0; i < heroes.size(); ++i) {
      const Character& h = heroes[i];
      std::cout << "  [" << (i + 1) << "] " << h.name << "\n"
                << "      ATK " << h.baseAtk << "  DEF " << h.baseDef << "  HP "
                << h.baseMaxHp << "  LUCK " << std::fixed << std::setprecision(2)
                << h.baseLuck << std::defaultfloat << "\n"
                << "      Skill: " << h.skillName << " - " << h.skillDesc << "\n";
      const auto model = heroModel(h.name);
      for (const auto& line : model) std::cout << "      " << line << "\n";
      std::cout << "\n";
    }
    std::cout << "\nEnter a number to choose a hero: ";
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
      std::cout << "\nYou selected: " << selected.name << "\n";
      pause();
      return selected;
    }
    std::cout << "Invalid input, please enter 1-" << heroes.size() << " .\n";
    pause();
  }
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

void applyHeroVictoryPassive(GameState& g, std::vector<std::string>& log) {
  if (g.hero.name.find("TEXT_17") != std::string::npos) {
    int before = g.hero.hp;
    g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 12);
    int healed = g.hero.hp - before;
    if (healed > 0) {
      std::ostringstream ss;
      ss << "  [TEXT_18] " << g.hero.skillName << "：Recover after battle " << healed << " HP";
      log.push_back(ss.str());
    }
  }
}

}
