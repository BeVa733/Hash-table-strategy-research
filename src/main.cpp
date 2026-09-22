#include "../include/Cache.hpp"

int main() {
  try {
    MultiLevelCache<int, int> CacheSystem("config.txt");

    int Key;
    while (std::cin >> Key) {
      CacheSystem.accessElement(Key);

      // IdealCacheSystem.access(Key);
    }

    std::cout << "Hit count = " << CacheSystem.HitCount_ << '\n';
    std::cout << "Miss count = " << CacheSystem.MissCount_ << '\n';

  } catch (const std::exception &Exception) {
    std::cout << Exception.what() << '\n';
  }

  return 0;
}
