#pragma once

#include "../policies/LRU.hpp"
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
  /// Vetor of cache levels ordered according to lookup priority.
  using CacheLevelVector =
      std::vector<std::unique_ptr<CacheLevel<ValueType, KeyType>>>;

  MultiLevelCache(const char *ConfigFilename) {

    using PolicyPtr = std::unique_ptr<CachePolicy<KeyType>>;

    // this hash map make connection between string policy names and class
    // costructors. Append new policies here after impementation
    std::unordered_map<std::string, std::function<PolicyPtr()>> Constructors{
        {"LRU", [] { return std::make_unique<LRUPolicy<KeyType>>(); }},
        {"FIFO", [] { return nullptr; }}};

    std::ifstream ConfigFile(ConfigFilename);
    if (!ConfigFile) {
      return;
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

      // get policy class pointer
      std::unique_ptr<CachePolicy<KeyType>> LevelPolicy = It->second();

      // get level capacity from stdin
      std::size_t LevelCapacity;
      std::cin >> LevelCapacity;

      std::unique_ptr<CacheLevel<ValueType, KeyType>> Level =
          std::make_unique(CacheLevel(LevelCapacity, LevelPolicy));

      CacheLevels_.emplace_back(Level);
    }
  }

  /// Return a cached value for \p Key or std::nullopt after a cache miss.
  std::optional<ValueType> accessElement(KeyType Key);

private:
  /// Cache levels number
  std::size_t NumLevels_;

  /// Cache levels searched by access().
  CacheLevelVector CacheLevels_;

public:
  /// Number of successful lookups.
  uint32_t CountHits_;
  /// Number of unsuccessful lookups.
  uint32_t CountMisses_;
};

template <typename ValueType, typename KeyType> class CacheLevel {

  /// A single cache level that stores values and applies one replacement
  /// policy.

public:
  CacheLevel(std::size_t Capacity, std::unique_ptr<CachePolicy<KeyType>> Policy)
      : Capacity_(Capacity), Policy_(std::move(Policy)) {}

  /// Return the value associated with \p Key or std::nullopt if it exists.
  std::optional<ValueType> findElement(const KeyType &Key);

  /// Insert \p Key with \p Value in cache and notify the cache policy
  void insertElement(const KeyType &Key, const ValueType *Value);

  /// Remove \p Key and notify the cache policy after that
  void eraseElement(const KeyType &Key);

private:
  /// Max number of elements in this cache level
  std::size_t Capacity_;

  /// Values currently stored in this cache level.
  std::unordered_map<KeyType, ValueType *> Map_;

  /// Replacement policy used when this level.
  std::unique_ptr<CachePolicy<KeyType>> Policy_;
};
