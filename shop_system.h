#include "game_data.h"

namespace Game {

Equipment::Equipment(std::string id_, std::string name_, std::string desc_, int atk_, int def_,
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
  maxStamina = std::min(100, 40 + maxHp / 6 + def / 3);
  maxStamina = std::max(24, maxStamina);
  stamina = std::min(stamina, maxStamina);
}

bool Character::hasBerserkerAxe() const {
  for (const auto& it : items) {
    if (it.special == EquipSpecial::BerserkerAxe) return true;
  }
  return false;
}

}
