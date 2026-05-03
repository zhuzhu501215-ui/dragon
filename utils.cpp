// Utility implementations.

#include "utils.h"

namespace Game {

std::mt19937& rng() {
  static std::mt19937 gen(static_cast<unsigned>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  return gen;
}

// Uniform random in [0.0, 1.0).
double rand01() {
  std::uniform_real_distribution<double> d(0.0, 1.0);
  return d(rng());
}

// Uniform random integer in inclusive range [a, b].
int randInt(int a, int b) {
  std::uniform_int_distribution<int> d(a, b);
  return d(rng());
}

void pause() {
  std::cout << "\n[ Press Enter to continue... ]";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Repeat character c n times (for box borders).
std::string repeat(char c, int n) {
  return std::string(std::max(0, n), c);
}

// ANSI clear screen and move cursor to top-left.
void clearScreen() {
  std::cout << "\x1B[2J\x1B[H";
}

}
