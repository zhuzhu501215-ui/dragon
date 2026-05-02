#ifndef UTILS_H
#define UTILS_H

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

std::mt19937& rng();
double rand01();
int randInt(int a, int b);
void pause();
std::string repeat(char c, int n);
void clearScreen();

}

#endif
