#include <iostream>
#include <limits>
#include <string>

#include "ui_flow.h"
#include "hero_system.h"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  Game::printStartScreen();
  Game::mainMenu();
  Game::GameState g;
  Game::printHeader("Tutorial Mode");
  std::cout << "Enable beginner tutorial? [Y/n]: ";
  std::string tutorialChoice;
  if (std::getline(std::cin, tutorialChoice)) {
    g.tutorialMode =
        tutorialChoice.empty() || tutorialChoice[0] == 'y' || tutorialChoice[0] == 'Y';
  }
  if (g.tutorialMode) Game::printTutorialIntro();
  g.hero = Game::chooseHero();
  std::cout << "Press Enter to start the adventure...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  while (g.stage <= 9) {
    Game::printTutorialMainMenuHint(g);
    Game::printHeader("TEXT_110 — Stage " + std::to_string(g.stage) + "TEXT_111");
    Game::printHeroPanel(g);
    std::cout << "\n  [1] Enter battle (progress stage)\n"
              << "  [2] Open shop\n"
              << "  [3] View rules\n"
              << "  [0] Quit game\n"
              << "\nChoose: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;
    if (cmd == "0") {
      std::cout << "Goodbye!\n";
      return 0;
    }
    if (cmd == "3") {
      Game::printHeader("Rules");
      std::cout
          << "  · TEXT_112：HP∈[0,300]，TEXT_113/DefendTEXT_114，TEXT_115 .\n"
          << "  · stamina STA：TEXT_116DefendTEXT_117；TEXT_118 .\n"
          << "    TEXT_119 [1] AttackCost 12 TEXT_120stamina；[2] DefendTEXT_121，TEXT_122，TEXT_123 14 TEXT_120stamina .\n"
          << "  · TEXT_36·TEXT_124「TEXT_125」：TEXT_126 HP>20，TEXT_127 20 HP TEXT_128 .\n"
          << "  · TEXT_129（TEXT_130） .\n"
          << "  · TEXT_131 3 TEXT_93；Stage 3、6、9Stage 2 TEXT_132 Boss，TEXT_133 .\n"
          << "  · Difficulty Curve：TEXT_134；TEXT_135 .\n"
          << "  · TEXT_136Hell Mode（TEXT_3 x1.5 / TEXT_3 x2.0 / Boss xTEXT_137） .\n\n";
      Game::pause();
      continue;
    }
    if (cmd == "2") {
      Game::shopLoop(g);
      continue;
    }
    if (cmd != "1") continue;
    if (!Game::runStage(g)) {
      Game::printHeader("Game Over");
      std::cout << "TEXT_50Score: " << g.score << "  Rank: " << Game::gradeFromScore(g.score) << "\n";
      Game::printEndingScreen(false, g);
      Game::pause();
      return 0;
    }
    ++g.stage;
    if (g.stage > 9) {
      g.normalCleared = true;
      Game::printHeader("TEXT_138Cleared!");
      Game::printHeroPanel(g);
      std::cout << "\nFinal rank: " << Game::gradeFromScore(g.score) << "  Score: " << g.score << "\n";
      Game::printEndingScreen(true, g);
      Game::pause();
      break;
    }
    std::cout << "\nTEXT_139 .\nTEXT_140？[y/N]: ";
    std::string yn;
    if (std::getline(std::cin, yn) && !yn.empty() && (yn[0] == 'y' || yn[0] == 'Y')) {
      Game::shopLoop(g);
    }
  }
  while (g.normalCleared) {
    Game::printHeader("Post-clear Menu");
    std::cout << "  [1] TEXT_141Hell Mode\n";
    if (g.hellCleared) {
      std::cout << "  [2] Set final Boss multiplier (current x" << std::fixed << std::setprecision(2)
                << g.customBossMultiplier << std::defaultfloat << ")\n";
    }
    std::cout << "  [0] Quit game\n\nChoose: ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;
    if (cmd == "0") break;
    if (cmd == "1") {
      Game::runHellMode(g);
    } else if (cmd == "2" && g.hellCleared) {
      g.customBossMultiplier = Game::askBossMultiplier(g.customBossMultiplier);
    }
  }
  return 0;
}
