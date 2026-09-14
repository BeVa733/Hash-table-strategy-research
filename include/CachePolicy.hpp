#pragma once

#include <string>

template <typename KeyType> class CachePolicy {

  /// Template class for policies, all methodes is virtual
  /// if you need to add new cache policy you need to override all of
  /// CachePolicy API

public:
  /// Return policy name for debug or beauty output
  virtual std::string getPolicyName() = 0;

  /// Return true if pollicy need insert \p Key into Cache after miss. Most
  /// likely return true in majority of policies
  virtual bool needInsertInCache(KeyType Key) = 0;

  /// Make policy react for insertion of element into cache. Use it if you
  /// insert \p Key in cache manually
  virtual void onCacheInsert(KeyType Key) = 0;

  /// Make policy react for hit \p Key element in cache. Implementation of
  /// hidden policy logic
  virtual void onCacheHit(KeyType Key) = 0;

  /// Make policy react for erase \p Key element from cache. Implementation of
  /// hidden policy logic. Use it if you erase element manually
  virtual void onCacheErase(KeyType Key) = 0;

  /// Return Key of element that need to erase. Use it if haven't enough space
  /// in cache
  virtual KeyType selectVictim(void) = 0;

  virtual ~CachePolicy() = default;
};
