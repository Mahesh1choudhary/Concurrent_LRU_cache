#ifndef HASH_NODE_H_
#define HASH_NODE_H_

namespace CHML{

template<typename Key, typename Value> class HashNode
{
public:
    HashNode *next = nullptr;   // pointer to next node in the same bucket
    HashNode()
    {

    }

    HashNode(Key key_, Value value_):key(key_), value(value_)
    {
    }
    ~HashNode()
    {
        next= nullptr;
    }

    // deleting copyconstructors, = operator, etc to maintain thread safety
    HashNode(const HashNode&) =delete;
    HashNode(HashNode&&) = delete;
    HashNode& operator=(const HashNode&) =delete;
    HashNode& operator=(HashNode&&)= delete;

    const Key &getKey() const
    {
        return key;
    }

    void setValue(Value value_){
        value=value_;
    }

    const Value &getValue() const
    {
        return value;
    }

private:
    Key key;    // hashed key
    Value value;    // value corresponding to key
};

}

#endif