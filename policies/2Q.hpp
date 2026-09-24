#pragma once

#include <iostream>
#include <unordered_map>
#include <list>
#include <algorithm>

#include "../include/CachePolicy.hpp"

template <typename KeyType> class TwoQPolicy : public CachePolicy<KeyType> {
public:
    TwoQPolicy(std::size_t Size) : Capacity_(Size), A1Capacity_(std::max<std::size_t>(1, Size / 4)) {} //25% to A1Capacity

    std::string getPolicyName() override { return "2Q"; }

    bool needInsertInCache(const KeyType &Key) override { return true; }

    void onCacheInsert(const KeyType &Key) override {
        if (A1In_.size() + Am_.size() >= Capacity_) {
            std::cerr << "[2Q] Error: try to insert extra element (corrupted)\n";
            return;
        }
        A1In_.push_front(Key);
    }

    void onCacheHit(const KeyType &Key) override {
        auto It = std::find(Am_.begin(), Am_.end(), Key);
        if (It != Am_.end()) {
            Am_.splice(Am_.begin(), Am_, It);
            return;
        }

        It = std::find(A1In_.begin(), A1In_.end(), Key);
        if (It != A1In_.end()) {
            Am_.splice(Am_.begin(), A1In_, It);
            return;
        }

        std::cerr << "[2Q] Error: hit on unknown key (corrupted)\n";
    }

    void onCacheErase(const KeyType &Key) override {
        auto It = std::find(A1In_.begin(), A1In_.end(), Key);
        if (It != A1In_.end()) {
            A1In_.erase(It);
            return;
        }

        It = std::find(Am_.begin(), Am_.end(), Key);
        if (It != Am_.end()) {
            Am_.erase(It);
            return;
        }

        std::cerr << "[2Q] Error: try to delete undefined element\n";
    }

    std::optional<KeyType> selectVictim() override {
        if (A1In_.size() > A1Capacity_) {
            return A1In_.back();
        }
        if (!Am_.empty()) {
            return Am_.back();
        }
        if (!A1In_.empty()) {
            return A1In_.back();
        }

        std::cerr << "[2Q] Warning: try to search victim in empty cache\n";
        return std::nullopt;
    }



private:
    std::list<KeyType> A1In_; // новички, FIFO
    std::list<KeyType> Am_;   // горячие, LRU

    std::size_t Capacity_;   // всего мест
    std::size_t A1Capacity_; // квота новичков
};
