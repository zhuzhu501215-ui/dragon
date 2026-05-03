#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

// 终端界面：标题框、进度条与英雄状态面板

#include "game_data.h"

namespace Game {

// 打印双线框标题
void printHeader(const std::string& t);

// 打印一行进度条（如 HP/体力）
void printBar(const std::string& title, int cur, int max, char fill, int width);

// 打印英雄 ASCII 面板（属性、装备、祝福、金币等）
void printHeroPanel(const GameState& g);

}

#endif
