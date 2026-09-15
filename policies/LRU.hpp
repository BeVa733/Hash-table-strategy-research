#pragma once

#include <iostream>

#include "../include/CachePolicy.hpp"

template <typename KeyType> class LRUPolicy : CachePolicy<KeyType> {

  LRUPolicy() {};

  std::string getPolicyName() override {
    std::cerr << "Not impemented\n";
    return "\n";
  }

  bool needInsertInCache(const KeyType &Key) override {
    std::cerr << "Not impemented\n";
    return true;
  }

  void onCacheInsert(const KeyType &Key) override {
    std::cerr << "Not impemented\n";
  }

  void onCacheHit(const KeyType &Key) override {
    std::cerr << "Not impemented\n";
  }

  void onCacheErase(const KeyType &Key) override {
    std::cerr << "Not impemented\n";
  }

  std::optional<KeyType> selectVictim(void) override {
    std::cerr << "Not impemented\n";
    return std::nullopt;
  }
};
