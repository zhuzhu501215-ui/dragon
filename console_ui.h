#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

// Terminal UI: framed headers, progress bars, and hero status panel.

#include "game_data.h"

namespace Game {

// Print a double-line framed title.
void printHeader(const std::string& t);

// Print one text progress bar (e.g. HP or stamina).
void printBar(const std::string& title, int cur, int max, char fill, int width);

// Print hero ASCII panel (stats, gear, blessings, gold, etc.).
void printHeroPanel(const GameState& g);

}

#endif
