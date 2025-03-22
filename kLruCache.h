#pragma once

#include "LruBase.h"

namespace MyCache 
{

template<typename Key, typename Value>
class KLruCache : public LruBase<Key, Value> {
public:
    KLruCache(int capacity, int historyCapacity, int k) 
    : LruBase<Key, Value> (capacity)
    , historyList_(std::make_unique<LruBase<Key, size_t>> (historyCapacity))
    , k_(k)
    {}

    Value get(Key key) {
        int historyCount = historyList_->get(key);
        historyList_->put(key, ++historyCount);
        return LruBase<Key, Value>::get(key);
    }

    void put(Key key, Value value) {
        if (LruBase<Key, Value>::get(key) != "") LruBase<Key, Value>::put(key, Value);
        int historyCount = historyList_->get(key);
        historyList_->put(key, historyCount ++);

        if (historyCount >= k_) {
            historyList_->remove(key);
            LruBase<Key, Value>::put(key, value);
        }
    }

private:
    int                                   k_;
    std::unique_ptr<LruBase<key, size_t>> historyList_;
};
}