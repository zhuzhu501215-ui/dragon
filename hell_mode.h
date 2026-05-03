#ifndef HELL_MODE_H
#define HELL_MODE_H

// 地狱模式三连战与根据总分评级的字母档

#include "game_data.h"

namespace Game {

char gradeFromScore(int s);

bool runHellMode(GameState& g);

}

#endif
