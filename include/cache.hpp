#pragma once

#include "CachePolicy.hpp"

#include <cstdint>
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

  /// Return a cached value for \p Key or std::nullopt after a cache miss.
  std::optional<ValueType> accessElement(KeyType Key);

private:
  /// Cache levels searched by access().
  CacheLevelVector CacheLevels;

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
  /// Return the value associated with \p Key or std::nullopt if it exists.
  std::optional<ValueType> findElement(KeyType Key);

  /// Insert \p Key with \p Value in cache and notify the cache policy
  void insertElement(KeyType Key, ValueType Value);

  /// Remove \p Key and notify the cache policy after that
  void eraseElement(KeyType Key);

private:
  /// Values currently stored in this cache level.
  std::unordered_map<KeyType, ValueType> map_;

  /// Replacement policy used when this level.
  std::unique_ptr<CachePolicy<KeyType>> policy_;
};
