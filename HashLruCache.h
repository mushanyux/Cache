#pragma once
#include "LruBase.h"

namespace MyCache {

template<typename Key, typename Value>
class HashLruCaches {
public:
    HashLruCaches(size_t capacity, int sliceNum)
        : capacity_(capacity)
        , sliceNum_(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency())
    {
        size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum_));
        for (int i = 0; i < sliceNum_; i ++) {
            lruSliceCaches_.emplace_back(new LruBase<Key, Value> (sliceSize));
        }
    }

    void put(Key key, Value value) {
        size_t sliceIndex = Hash(key) % sliceNum_;
        return lruSliceCaches_[sliceIndex]->put(key, value);
    }

    bool get(Key key, Value& value) {
        size_t sliceIndex = Hash(key) % sliceNum_;
        return lruSliceCaches_[sliceIndex]->get(key, value);
    }

    Value get(Key key) {
        Value value;
        memset(&value, 0, sizeof Value);
        get(key, value);
        return value;
    }
private:
    size_t Hash(Key key) {
        std::hash<Key> hashFunc;
        return hashFunc(key);
    } 
    size_t                                              capacity_;
    int                                                 sliceNum_;
    std::vector<std::unique_ptr<LruBase<Key, Value>>>   lruSliceCaches_;
};
}