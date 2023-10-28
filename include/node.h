#ifndef NODE_H
#define NODE_H 1

#include <ostream>
#include "def.h"

enum struct ChildType : bool
{
    INDEX,
    LEAF
};

template <class KeyType, size_type MaxKeys>
struct Node
{
    KeyType keys[MaxKeys];
    size_type key_count = 0;
};

template <class KeyType, size_type MaxKeys>
struct IndexNode : Node<KeyType, MaxKeys>
{
    IndexNode(ChildType);

    ChildType child_type;
    Node<KeyType, MaxKeys> *children[MaxKeys + 1];
    size_type child_count = 0;
    IndexNode *father = nullptr;
};

template <class KeyType, size_type MaxKeys>
struct LeafNode : Node<KeyType, MaxKeys>
{
    LeafNode *next = nullptr;
};

template <class KeyType, size_type MaxKeys>
inline IndexNode<KeyType, MaxKeys>::IndexNode(ChildType childType) : child_type(childType)
{
}

template <class KeyType, size_type MaxKeys>
inline static std::ostream &operator<<(std::ostream &os, const Node<KeyType, MaxKeys> &node)
{
    os << '[';

    for (size_type i = 0; i < node.key_count; ++i)
        i ? os << ' ' << node.keys[i] : os << node.keys[i];

    return os << ']';
}

template <class KeyType, size_type MaxKeys>
inline static std::ostream &operator<<(std::ostream &os, const IndexNode<KeyType, MaxKeys> &inode)
{
    return os << static_cast<Node<KeyType, MaxKeys>>(inode);
}

template <class KeyType, size_type MaxKeys>
inline static std::ostream &operator<<(std::ostream &os, const LeafNode<KeyType, MaxKeys> &lnode)
{
    return os << static_cast<Node<KeyType, MaxKeys>>(lnode);
}

#endif
