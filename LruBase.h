#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include "CacheSer.h"

namespace MyCache 
{
template<typename Key, typename Value> class LruBase;

template<typename Key, typename Value>
class LruNode 
{
private:
    Key key_;
    Value value_;
    size_t accessCount_; 
    std::shared_ptr<LruNode<Key, Value>> prev_;  
    std::shared_ptr<LruNode<Key, Value>> next_;

public:
    LruNode(Key key, Value value)
        : key_(key)
        , value_(value)
        , accessCount_(1) 
        , prev_(nullptr)
        , next_(nullptr)
    {}

    Key getKey() const { return key_; }
    Value getValue() const { return value_; }
    void setValue(const Value& value) { value_ = value; }
    size_t getAccessCount() const { return accessCount_; }
    void incrementAccessCount() { ++accessCount_; }

    friend class LruBase<Key, Value>;
};


template<typename Key, typename Value>
class LruBase : public CacheSer<Key, Value>
{
public:
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

    LruBase(int capacity)
        : capacity_(capacity)
    {
        initializeList();
    }

    ~LruBase() override = default;

    void put(Key key, Value value) override
    {
        if (capacity_ <= 0)
            return;
    
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end())
        {
            updateExistingNode(it->second, value);
            return ;
        }

        addNewNode(key, value);
    }

    bool get(Key key, Value& value) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end())
        {
            moveToMostRecent(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }

    Value get(Key key) override
    {
        Value value{};
        get(key, value);
        return value;
    }

    void remove(Key key) 
    {   
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end())
        {
            removeNode(it->second);
            nodeMap_.erase(it);
        }
    }

private:
    void initializeList()
    {
        dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyHead_->next_ = dummyTail_;
        dummyTail_->prev_ = dummyHead_;
    }

    void updateExistingNode(NodePtr node, const Value& value) 
    {
        node->setValue(value);
        moveToMostRecent(node);
    }

    void addNewNode(const Key& key, const Value& value) 
    {
       if (nodeMap_.size() >= capacity_) 
       {
           evictLeastRecent();
       }

       NodePtr newNode = std::make_shared<LruNodeType>(key, value);
       insertNode(newNode);
       nodeMap_[key] = newNode;
    }

    void moveToMostRecent(NodePtr node) 
    {
        removeNode(node);
        insertNode(node);
    }

    void removeNode(NodePtr node) 
    {
        node->prev_->next_ = node->next_;
        node->next_->prev_ = node->prev_;
    }

    void insertNode(NodePtr node) 
    {
        node->next_ = dummyTail_;
        node->prev_ = dummyTail_->prev_;
        dummyTail_->prev_->next_ = node;
        dummyTail_->prev_ = node;
    }

    void evictLeastRecent() 
    {
        NodePtr leastRecent = dummyHead_->next_;
        removeNode(leastRecent);
        nodeMap_.erase(leastRecent->getKey());
    }

private:
    int          capacity_; 
    NodeMap      nodeMap_; 
    std::mutex   mutex_;
    NodePtr      dummyHead_; 
    NodePtr      dummyTail_;
};
}