#include "../include/Cache.hpp"

int main() {

  MultiLevelCache<int, int> CacheSystem("config.txt");

  int Key;
  while (std::cin >> Key) {
    CacheSystem.accessElement(Key);

    // IdealCacheSystem.access(Key);
  }

  std::cout << "Hit count = " << CacheSystem.HitCount_ << '\n';
  std::cout << "Miss count = " << CacheSystem.MissCount_ << '\n';

  return 0;
}
