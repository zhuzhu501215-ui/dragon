#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include "game_data.h"

namespace Game {

void printHeader(const std::string& t);

void printBar(const std::string& title, int cur, int max, char fill, int width);

void printHeroPanel(const GameState& g);

}

#endif
