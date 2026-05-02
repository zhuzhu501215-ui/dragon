#include "shop_system.h"
#include "console_ui.h"
#include "hero_system.h"
#include "utils.h"

namespace Game {

void printTutorialShopHint(GameState& g);

const Equipment kShopCatalog[] = {
    {"wood_sword", "[Weapon] Wooden Sword", "ATK +5", 5, 0, 0, 0.0, EquipSpecial::None, 40},
    {"iron_armor", "[Armor] Iron Armor", "DEF +8", 0, 8, 0, 0.0, EquipSpecial::None, 55},
    {"ruby", "[Gem] Ruby", "Max HP +25", 0, 0, 25, 0.0, EquipSpecial::None, 45},
    {"clover", "[Luck] Four-leaf Clover", "Luck +0.05", 0, 0, 0, 0.05, EquipSpecial::None, 50},
};

void shopLoop(GameState& g) {
  while (true) {
    printTutorialShopHint(g);
    printHeader("Shop");
    printHeroPanel(g);
    std::cout
        << "\n  [1] Buy Equipment\n"
        << "  [2] Buy Blessing\n"
        << "  [3] Spend 60 gold: Star-up Vane Hellion (->2 stars, unlock Berserker Axe)\n"
        << "  [4] Spend 25 gold: Recover 40 HP (up to max)\n"
        << "  [0] Leave Shop\n"
        << "\nChoose: ";
    std::string line;
    if (!std::getline(std::cin, line)) return;
    if (line == "0") break;
    if (line == "1") {
      std::cout << "\nAvailable equipment:\n";
      for (size_t i = 0; i < sizeof(kShopCatalog) / sizeof(kShopCatalog[0]); ++i) {
        const auto& e = kShopCatalog[i];
        std::cout << "  [" << (i + 1) << "] " << e.name << " — " << e.desc << " - Price " << e.price
                  << "\n";
      }
      std::cout << "Enter number to buy, 0 to return: ";
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
        std::cout << "Not enough gold.\n";
        pause();
        continue;
      }
      g.gold -= e.price;
      g.hero.items.push_back(e);
      g.hero.recomputeStats();
      g.hero.hp = std::min(g.hero.hp, g.hero.maxHp);
      ensureVaneTwoStarGear(g);
      std::cout << "Purchased " << e.name << "\n";
      pause();
    } else if (line == "2") {
      std::cout << "\nBlessing list:\n"
                << "  [1] War God Blessing — ATK+5 — 55 TEXT_32\n"
                << "  [2] Guardian Blessing — DEF+6 — 55 TEXT_32\n"
                << "  [3] Life Blessing — TEXT_33 HP+30 — 60 TEXT_32\n"
                << "  [4] Lucky Blessing — LUCK+0.03 — 65 TEXT_32\n"
                << "0 TEXT_34\nTEXT_35: ";
      std::string pick;
      if (!std::getline(std::cin, pick)) return;
      Blessing b;
      int cost = 0;
      if (pick == "1") {
        b = {"War God Blessing", "ATK+5", 5, 0, 0, 0};
        cost = 55;
      } else if (pick == "2") {
        b = {"Guardian Blessing", "DEF+6", 0, 6, 0, 0};
        cost = 55;
      } else if (pick == "3") {
        b = {"Life Blessing", "Max HP +30", 0, 0, 30, 0};
        cost = 60;
      } else if (pick == "4") {
        b = {"Lucky Blessing", "LUCK+0.03", 0, 0, 0, 0.03};
        cost = 65;
      } else {
        continue;
      }
      if (g.gold < cost) {
        std::cout << "Not enough gold.\n";
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
      std::cout << "Gained blessing: " << b.name << "\n";
      pause();
    } else if (line == "3") {
      if (g.hero.name.find("TEXT_36·TEXT_37") == std::string::npos) {
        std::cout << "Only Vane Hellion can star-up.\n";
        pause();
        continue;
      }
      if (g.hero.stars >= 2) {
        std::cout << "Already 2 stars or above.\n";
        pause();
        continue;
      }
      if (g.gold < 60) {
        std::cout << "Not enough gold.\n";
        pause();
        continue;
      }
      g.gold -= 60;
      g.hero.stars = 2;
      ensureVaneTwoStarGear(g);
      std::cout << "Star-up success! Gained exclusive equipment \"Berserker Axe\".\n";
      pause();
    } else if (line == "4") {
      if (g.gold < 25) {
        std::cout << "Not enough gold.\n";
        pause();
        continue;
      }
      g.gold -= 25;
      g.hero.hp = std::min(g.hero.maxHp, g.hero.hp + 40);
      std::cout << "HP recovered.\n";
      pause();
    }
  }
}

}
