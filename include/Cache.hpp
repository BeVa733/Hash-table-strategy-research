#pragma once

#include "../policies/LRU.hpp"
#include "../policies/LFU.hpp"
#include "CachePolicy.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

/// Forward declaration of a cache level used by MultiLevelCache.
template <typename ValueType, typename KeyType> class CacheLevel;

template <typename ValueType, typename KeyType> class MultiLevelCache {

  /// Class that coordinates lookup and movement of elements across cache
  /// levels.

public:
  /// Vector of cache levels ordered according to lookup priority.
  using CacheLevelVector =
      std::vector<std::unique_ptr<CacheLevel<ValueType, KeyType>>>;

  /// Constructor with reading config file
  MultiLevelCache(const char *ConfigFilename) {

    using PolicyPtr = std::unique_ptr<CachePolicy<KeyType>>;

    // this hash map make connection between string policy names and class
    // costructors. Append new policies here after impementation
    std::unordered_map<std::string, std::function<PolicyPtr(std::size_t)>>
        Constructors{{"LRU",
                      [](std::size_t Capacity) {
                        return std::make_unique<LRUPolicy<KeyType>>(Capacity);
                      }},
                     {"FIFO", [](std::size_t Capacity) { return nullptr; }}};

    std::ifstream ConfigFile(ConfigFilename);
    if (!ConfigFile) {
      std::cerr << "FATAL: Unable to open config file: " << ConfigFilename
                << '\n';
      return; // TODO replace all return with exceptions
    }

    // Read number of cache levels
    if (!(ConfigFile >> NumLevels_)) {
      std::cerr << "FATAL: Incorrect config format. At first must be number of "
                   "cache levels\n";
      return;
    }

    // read policy names in cycle
    for (int i = 0; i < NumLevels_; ++i) {
      std::string PolicyName;
      if (!(ConfigFile >> PolicyName)) {
        std::cerr << "FATAL: Incorrect config format. Unexpected symbol "
                     "instead of policy name\n";
        return;
      }

      // find policy in hash map
      auto It = Constructors.find(PolicyName);
      if (It == Constructors.end()) {
        std::cerr << "FATAL: Unknown name of policy in config file\n";
        return;
      }

      // get level capacity from stdin
      std::size_t LevelCapacity;
      std::cin >> LevelCapacity;

      // Add cache level into levels vector
      CacheLevels.emplace_back(std::make_unique<CacheLevel<ValueType, KeyType>>(
          LevelCapacity, It->second(LevelCapacity)));
    }
  }

  /// Return a cached value for \p Key or std::nullopt after a cache miss.
  std::optional<ValueType *> accessElement(KeyType Key) {
    std::optional<ValueType *> Value = std::nullopt;

    // use & because unique ptr cannot be copied
    for (auto &CL : CacheLevels) {
      Value = CL->findElement(Key);

      if (Value.has_value()) {

        CL->Policy->onCacheHit(Key);
        ++HitCount_;
        return Value;

      } else {

        Value = slowGetPage(Key);

        if (CL->hasFreeSpace()) {

          CL->insertElement(Key, *Value);
          CL->Policy->onCacheInsert(Key);

          return Value;
        }

        if (CL->Policy->needInsertInCache(Key)) {

          std::optional<KeyType> VictimElement = CL->Policy->selectVictim();

          if (VictimElement.has_value()) {
            CL->eraseElement(*VictimElement);
            CL->Policy->onCacheErase(*VictimElement);
          }
        }
      }
    }

    return std::nullopt;
  }

  /// Function for immitate slow process of getting value
  ValueType *slowGetPage(KeyType Key) { return nullptr; }

private:
  /// Cache levels number
  std::size_t NumLevels_;

  /// Cache levels searched by access().
  CacheLevelVector CacheLevels;

public:
  /// Number of successful lookups.
  uint32_t HitCount_;
  /// Number of unsuccessful lookups.
  uint32_t MissCount_;
};

template <typename ValueType, typename KeyType> class CacheLevel {

  /// A single cache level that stores values and applies one replacement
  /// policy.

public:
  CacheLevel(std::size_t Capacity, std::unique_ptr<CachePolicy<KeyType>> Policy)
      : Capacity_(Capacity), Policy(std::move(Policy)) {}

  /// Return the value associated with \p Key or std::nullopt if it exists.
  std::optional<ValueType *> findElement(const KeyType &Key) {

    auto It = Map.find(Key);

    if (It != Map.end()) {
      return It->second;
    } else {
      return std::nullopt;
    }
  }

  /// Insert \p Key with \p Value in cache and notify the cache policy
  void insertElement(const KeyType &Key, ValueType *Value) {

    if (Map.find(Key) == Map.end()) {
      Map.emplace(Key, Value);
    } else {
      std::cerr
          << "[CACHE] Warning: try to insert new element with existing key\n";
    }
  }

  /// Remove \p Key and notify the cache policy after that
  void eraseElement(const KeyType &Key) { Map.erase(Key); }

  /// Return true if is enough space for one element in Cache
  bool hasFreeSpace(void) { return Map.size() < Capacity_; }

private:
  /// Max number of elements in this cache level
  std::size_t Capacity_;

  /// Values currently stored in this cache level
  std::unordered_map<KeyType, ValueType *> Map;

public:
  /// Replacement policy used when this level.
  std::unique_ptr<CachePolicy<KeyType>> Policy;
};
