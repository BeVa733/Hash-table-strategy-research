#pragma once

#include <algorithm>
#include <iostream>
#include <list>

#include "../include/CachePolicy.hpp"

template <typename KeyType> class LRUPolicy : CachePolicy<KeyType> {

public:
  LRUPolicy(std::size_t Size) : Capacity_(Size) {};

  std::string getPolicyName() override { return "LRU"; }

  bool needInsertInCache(const KeyType &Key) override { return true; }

  void onCacheInsert(const KeyType &Key) override {
    if (List_.size() < Capacity_) {
      List_.push_front(Key);
    } else {
      std::cerr << "[LRU] Error: try to insert extra element (corrupted)\n";
    }
  }

  void onCacheHit(const KeyType &Key) override {
    if (List_.size() < Capacity_) {
      auto It = std::find(List_.begin(), List_.end(), Key);
      if (It != List_.end()) {
        List_.splice(List_.begin(), List_, It);
      } else {
        std::cerr
            << "[LRU] Error: try to splice undefined element (corrupted)\n";
      }
    } else {
      std::cerr << "[LRU] Error: try to splice extra element (corrupted)\n";
    }
  }

  void onCacheErase(const KeyType &Key) override {
    auto It = std::find(List_.begin(), List_.end(), Key);
		
    if (It != List_.end()) {
      List_.erase(It);
    } else {
      std::cerr << "[LRU] Error: try to delete undefined element \n";
    }
  }

  std::optional<KeyType> selectVictim(void) override {
    if (!List_.empty()) {
			return --(List_.end());
		} else {
			std::cerr << "[LRU] Warning: try to search victim in empty cache\n";
    	return std::nullopt;
		}
  }

private:
  std::list<KeyType> List_;

  std::size_t Capacity_;
};
