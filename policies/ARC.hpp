#pragma once

#include "../include/CachePolicy.hpp"

#include <algorithm>
#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>

template <typename KeyType>
class ARCPolicy : public CachePolicy<KeyType> {

private:

    enum class ListType { T1, T2, B1, B2 };

    struct Entry {
        ListType List;
        typename std::list<KeyType>::iterator Iterator;
    };

public:

    explicit ARCPolicy(std::size_t Capacity)
        : Capacity_(Capacity), P_(0) {}

    std::string getPolicyName() override {
        return "ARC";
    }

    bool needInsertInCache(const KeyType& Key) override {

        InsertAsT2_ = false;
        VictimGoesToGhost_ = false;
        LastMissWasFromB2_ = false;

        if (Capacity_ == 0) {
            return false;
        }

        auto It = Entries_.find(Key);

        if (It != Entries_.end()) {

            // Key comeback from B1.
            // Increase P_
            if (It->second.List == ListType::B1) {

                std::size_t Delta = B1_.empty()
                                  ? 1
                                  : std::max<std::size_t>(1, B2_.size() / B1_.size());

                P_ = std::min(Capacity_, P_ + Delta);
                removeEntry(Key);

                InsertAsT2_ = true;
                VictimGoesToGhost_ = true;
                return true;
            }

            // Key comeback from B2.
            // Decrease P_.
            if (It->second.List == ListType::B2) {

                std::size_t Delta = B2_.empty()
                                  ? 1
                                  : std::max<std::size_t>(1, B1_.size() / B2_.size());

                P_ = (P_ >= Delta) ? (P_ - Delta) : 0;
                removeEntry(Key);

                LastMissWasFromB2_ = true;
                InsertAsT2_ = true;
                VictimGoesToGhost_ = true;
                return true;
            }
        }

        std::size_t T1PlusB1 = T1_.size() + B1_.size();

        if (T1PlusB1 == Capacity_) {

            if (T1_.size() < Capacity_) {
                removeLRU(ListType::B1);
            }
        } else {

            std::size_t DirectorySize =
                T1_.size() + T2_.size() + B1_.size() + B2_.size();

            if (DirectorySize >= Capacity_) {

                if (DirectorySize == 2 * Capacity_) {
                    removeLRU(ListType::B2);
                }

                VictimGoesToGhost_ = true;
            }
        }

        return true;
    }

    void onCacheInsert(const KeyType& Key) override {

        if (Entries_.find(Key) != Entries_.end()) {
            return;
        }

        addToFront(Key, InsertAsT2_ ? ListType::T2 : ListType::T1);

        InsertAsT2_ = false;
        VictimGoesToGhost_ = false;
    }

    void onCacheHit(const KeyType& Key) override {

        auto It = Entries_.find(Key);
        if (It == Entries_.end()) {
            return;
        }

        if (It->second.List == ListType::T1) {
            moveToFront(Key, ListType::T1, ListType::T2);
        } else if (It->second.List == ListType::T2) {
            moveToFront(Key, ListType::T2, ListType::T2);
        }
    }

    void onCacheErase(const KeyType& Key) override {

        auto It = Entries_.find(Key);
        if (It == Entries_.end()) {
            return;
        }

        ListType CurrentList = It->second.List;

        if (CurrentList == ListType::T1) {
            if (VictimGoesToGhost_) {
                moveToFront(Key, ListType::T1, ListType::B1);
            } else {
                eraseEntry(Key);
            }
        } else if (CurrentList == ListType::T2) {
            if (VictimGoesToGhost_) {
                moveToFront(Key, ListType::T2, ListType::B2);
            } else {
                eraseEntry(Key);
            }
        }

        VictimGoesToGhost_ = false;
    }

    std::optional<KeyType> selectVictim(void) override {

        if (T1_.empty() && T2_.empty()) {
            return std::nullopt;
        }

        // Victim is from T1, if T1 more than target size,
        // or if last miss was from B2.
        if (!T1_.empty() &&
            (T1_.size() > P_ ||
             (LastMissWasFromB2_ && T1_.size() == P_))) {
            return T1_.back();
        }

        if (!T2_.empty()) {
            return T2_.back();
        }

        // If T2 is empty, but T1 is not empty.
        return T1_.back();
    }

private:

    void addToFront(const KeyType& Key, ListType List) {
        std::list<KeyType>& TargetList = getList(List);
        TargetList.push_front(Key);
        Entries_.emplace(Key, Entry{List, TargetList.begin()});
    }

    void moveToFront(const KeyType& Key, ListType From, ListType To) {

        auto It = Entries_.find(Key);
        if (It == Entries_.end()) {
            return;
        }

        std::list<KeyType>& FromList = getList(From);
        std::list<KeyType>& ToList   = getList(To);

        // If From == To - splice reaplce elemnt in the begining of the same list. 
        // Update LRU-position.
        ToList.splice(ToList.begin(), FromList, It->second.Iterator);

        It->second.List = To;
        It->second.Iterator = ToList.begin();
    }

    void eraseEntry(const KeyType& Key) {

        auto It = Entries_.find(Key);
        if (It == Entries_.end()) {
            return;
        }

        std::list<KeyType>& List = getList(It->second.List);

        List.erase(It->second.Iterator);
        Entries_.erase(It);
    }

    void removeLRU(ListType List) {

        std::list<KeyType>& TargetList = getList(List);
        if (TargetList.empty()) {
            return;
        }

        KeyType Key = TargetList.back();   // Copy: after erase iterator invalid.
    }

    std::list<KeyType>& getList(ListType List) {

        switch (List) {
            case ListType::T1: return T1_;
            case ListType::T2: return T2_;
            case ListType::B1: return B1_;
            case ListType::B2: return B2_;
        }

        // Unreachable with correct usage, but we protect against it anyway.
        throw std::logic_error("ARCPolicy::getList: unknown ListType");
    }

private:

    std::list<KeyType> T1_;
    std::list<KeyType> T2_;
    std::list<KeyType> B1_;
    std::list<KeyType> B2_;

    std::unordered_map<KeyType, Entry> Entries_;

    std::size_t Capacity_;

    // Target size T1.
    // Increases on B1 miss, decreases on B2 miss.
    std::size_t P_;

    bool InsertAsT2_{false};
    bool VictimGoesToGhost_{false};
    bool LastMissWasFromB2_{false};
};