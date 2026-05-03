#ifndef HELL_MODE_H
#define HELL_MODE_H

// Hell mode gauntlet and letter grade from total score.

#include "game_data.h"

namespace Game {

char gradeFromScore(int s);

bool runHellMode(GameState& g);

}

#endif
