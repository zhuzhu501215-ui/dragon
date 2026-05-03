// 工具函数实现

#include "utils.h"

namespace Game {

std::mt19937& rng() {
  static std::mt19937 gen(static_cast<unsigned>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  return gen;
}

// [0.0, 1.0) 均匀随机
double rand01() {
  std::uniform_real_distribution<double> d(0.0, 1.0);
  return d(rng());
}

// 闭区间 [a, b] 整数随机
int randInt(int a, int b) {
  std::uniform_int_distribution<int> d(a, b);
  return d(rng());
}

void pause() {
  std::cout << "\n[ Press Enter to continue... ]";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 重复字符 n 次组成字符串（用于画框线）
std::string repeat(char c, int n) {
  return std::string(std::max(0, n), c);
}

// ANSI 清屏并把光标移到左上角
void clearScreen() {
  std::cout << "\x1B[2J\x1B[H";
}

}
