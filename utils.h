#ifndef UTILS_H
#define UTILS_H

// 随机数、暂停、字符串重复、清屏等通用工具

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace Game {

enum class EquipSpecial { None, BerserkerAxe };

// 全局 Mersenne Twister 随机引擎（惰性初始化）
std::mt19937& rng();
double rand01();
int randInt(int a, int b);
// 等待用户按 Enter（会清空当前行剩余输入）
void pause();
std::string repeat(char c, int n);
void clearScreen();

}

#endif
