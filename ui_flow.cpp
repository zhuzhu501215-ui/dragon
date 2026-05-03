// UI 流程：单关战斗循环、开场/结局 ASCII、分步教程提示

#include "ui_flow.h"
#include "console_ui.h"
#include "hero_system.h"
#include "monster_system.h"
#include "utils.h"

namespace Game {

bool runStage(GameState& g) {
  printHeader("Stage " + std::to_string(g.stage) + "");
  printHeroPanel(g);
  // 每第三关第二场为 Boss（精英）
  bool hasElite = (g.stage % 3 == 0);
  printTutorialStageHint(g, hasElite);
  std::cout << "\nThis stage has 3 consecutive monster encounters";
  if (hasElite) std::cout << "（Stage 2 TEXT_38）";
  std::cout << " .\n";
  pause();
  for (int i = 1; i <= 3; ++i) {
    Monster m;
    if (hasElite && i == 2)
      m = makeStageBoss(g.stage);
    else
      m = randomCommon();
    printHeader("Battle " + std::to_string(i) + "/3");
    CombatResult cr = runBattle(g, m);
    for (const auto& s : cr.log) std::cout << s << "\n";
    if (!cr.win) return false;
    printHeroPanel(g);
    pause();
  }
  std::cout << "\n*** Stage " << g.stage << " Cleared!***\n";
  // 通关奖励：金币与分数累加到 GameState
  int clearGold = stageClearGoldBonus(g.stage);
  g.gold += clearGold;
  std::cout << "[Stage Reward] Gold +" << clearGold << "\n";
  int stageScore = 90 + g.stage * 20;
  g.score += stageScore;
  std::cout << "[Stage Reward] Score +" << stageScore << "\n";
  char gr = gradeFromScore(g.score);
  std::cout << "Current rank reference: " << gr << " grade (score " << g.score << "）\n";
  printTutorialPostStageHint(g);
  return true;
}

// 显示规则摘要（与主线流程中的详细规则区分）
void mainMenu() {
  printHeader("Golden Spatula Style - Console Demo");
  std::cout << "  ActionsTEXT_54：TEXT_55 .\n"
            << " TEXT_56 9；TEXT_39 3TEXT_57（TEXT_58Stage 2 TEXT_59） .\n"
            << "  TEXT_60；Hero「TEXT_61」TEXT_62（TEXT_63）TEXT_64 .\n"
            << "  TEXT_65Hell Mode（3TEXT_66） .\n\n";
}

// 游戏标题 ASCII 横幅
void printStartScreen() {
  std::cout << "\n";
  std::cout << "==============================================================\n";
  std::cout << "   *****   *****  *      *****   *****  *   *  *      ***** \n";
  std::cout << "  *       *   *  *      *   *  *      **  *  *      *      \n";
  std::cout << "  *  ***  *   *  *      *   *  ****   * * *  *      ****   \n";
  std::cout << "  *    *  *   *  *      *   *  *      *  **  *      *      \n";
  std::cout << "   *****   *****  *****  *****  *****  *   *  *****  ***** \n";
  std::cout << "--------------------------------------------------------------\n";
  std::cout << "                 TACTIC BATTLE - CONSOLE EDITION             \n";
  std::cout << "--------------------------------------------------------------\n";
  std::cout << "  # Build your hero  # Beat stage bosses  # Reach rank S     \n";
  std::cout << "==============================================================\n";
  std::cout << "\n";
}

// 胜利或失败时的结算横幅与统计摘要
void printEndingScreen(bool win, const GameState& g) {
  std::cout << "\n";
  std::cout << "==============================================================\n";
  if (win) {
    std::cout << "   *   *   ***   *   *      *   *   ***   *   *   !!!       \n";
    std::cout << "    * *   *   *  *   *      * * *  *   *  **  *   !!!       \n";
    std::cout << "     *    *   *  *   *      * * *  *   *  * * *   !!!       \n";
    std::cout << "     *    *   *  *   *      ** **  *   *  *  **             \n";
    std::cout << "     *     ***    ***       *   *   ***   *   *   !!!       \n";
    std::cout << "--------------------------------------------------------------\n";
    std::cout << "                      CHAMPION CLEAR                          \n";
  } else {
    std::cout << "   *****   ***   *   *  *****       ***   *   *  *****      \n";
    std::cout << "  *       *   *  ** **  *          *   *  *   *  *          \n";
    std::cout << "  *  ***  *****  * * *  ****       *   *   * *   ****       \n";
    std::cout << "  *    *  *   *  *   *  *          *   *   * *   *          \n";
    std::cout << "   *****   *   *  *   *  *****       ***     *    *****      \n";
    std::cout << "--------------------------------------------------------------\n";
    std::cout << "                        GAME OVER                             \n";
  }
  std::cout << "--------------------------------------------------------------\n";
  std::cout << "  Hero : " << g.hero.name << "\n";
  std::cout << "  Score: " << g.score << "   Rank: " << gradeFromScore(g.score) << "\n";
  std::cout << "  Stage: " << std::min(9, g.stage) << "/9   Gold: " << g.gold << "\n";
  std::cout << "==============================================================\n";
  std::cout << "\n";
}

// 教程总览：属性含义与 Boss 出现关卡
void printTutorialIntro() {
  printHeader("Beginner Tutorial Enabled");
  std::cout
      << "  TEXT_67，TEXT_68 .\n"
      << "  TEXT_69：TEXT_70（HP/DEF）→ TEXT_71（ATK/LUCK） .\n"
      << "  Stage 3/6/9Stage 2 TEXT_72 Boss，TEXT_73 .\n";
  pause();
}

// 首次进入主菜单时提示各选项用途（仅一次）
void printTutorialMainMenuHint(GameState& g) {
  if (!g.tutorialMode || g.tutorialMainMenuHintShown) return;
  printHeader("Tutorial: Main Menu");
  std::cout
      << "  [1] TEXT_74：TEXT_75，TEXT_76 .\n"
      << "  [2] Open shop：TEXT_77 HP，TEXT_78 Boss TEXT_79 .\n"
      << "  [3] Rules：TEXT_80 .\n"
      << "  TEXT_81 .\n";
  g.tutorialMainMenuHintShown = true;
  pause();
}

// 首次进商店时的购买优先级建议
void printTutorialShopHint(GameState& g) {
  if (!g.tutorialMode || g.tutorialShopHintShown) return;
  printHeader("Tutorial: Shop");
  std::cout
      << "  New player priorities:\n"
      << "  1) TEXT_82：TEXT_83 / Life Blessing / TEXT_84 HP\n"
      << "  2) TEXT_85：TEXT_86 / War God Blessing\n"
      << "  3) TEXT_87：TEXT_88 / Lucky Blessing\n"
      << "  TEXT_89：TEXT_90，TEXT_91 .\n";
  g.tutorialShopHintShown = true;
  pause();
}

// 开战前通用提示；若本关有 Boss 则追加 Boss 说明（各触发一次）
void printTutorialStageHint(GameState& g, bool hasElite) {
  if (!g.tutorialMode) return;
  if (!g.tutorialStageHintShown) {
    printHeader("Tutorial: Before Battle");
    std::cout
        << "  TEXT_92 3 TEXT_93，TEXT_94 .\n"
        << "  TEXT_95 .\n"
        << "  damageTEXT_96：max(1, ATK - DEF)，TEXT_97 .\n";
    g.tutorialStageHintShown = true;
    pause();
  }
  if (hasElite && !g.tutorialBossHintShown) {
    printHeader("TEXT_98：Boss");
    std::cout
        << "  TEXT_58Stage 2 TEXT_72 Boss .\n"
        << "  Boss TEXT_99 .\n"
        << "  TEXT_100：TEXT_101 HP，TEXT_102 .\n";
    g.tutorialBossHintShown = true;
    pause();
  }
}

// 通关后一次性的资源与备战提示
void printTutorialPostStageHint(GameState& g) {
  if (!g.tutorialMode || g.tutorialPostStageHintShown) return;
  printHeader("Tutorial: Clear Summary");
  std::cout
      << "  TEXT_103 + TEXT_104 .\n"
      << "  TEXT_105，TEXT_106 .\n"
      << "  TEXT_107 Boss，TEXT_108 HP/DEF TEXT_109 .\n";
  g.tutorialPostStageHintShown = true;
  pause();
}

}
