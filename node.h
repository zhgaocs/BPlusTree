#pragma once
#include <ostream>

enum struct ChildType : bool
{
    INDEX,
    LEAF
};

template <class KeyType, std::size_t MaxKeys>
struct Node
{
    typedef KeyType key_type;
    typedef std::size_t size_type;

    KeyType keys[MaxKeys];
    size_type key_count = 0;
};

template <class KeyType, std::size_t MaxKeys>
struct IndexNode : Node<KeyType, MaxKeys>
{
    typedef KeyType key_type;
    typedef typename Node<KeyType, MaxKeys>::size_type size_type;

    IndexNode(ChildType);

    ChildType child_type;
    Node<KeyType, MaxKeys> *children[MaxKeys + 1];
    size_type child_count = 0;
    IndexNode *father = nullptr;
};

template <class KeyType, std::size_t MaxKeys>
struct LeafNode : Node<KeyType, MaxKeys>
{
    typedef KeyType key_type;
    typedef typename Node<KeyType, MaxKeys>::size_type size_type;

    LeafNode *next = nullptr;
};

template <class KeyType, std::size_t MaxKeys>
inline IndexNode<KeyType, MaxKeys>::IndexNode(ChildType c_type) : child_type(c_type)
{
}

template <class KeyType, std::size_t MaxKeys>
inline static std::ostream &operator<<(std::ostream &os, const Node<KeyType, MaxKeys> &node)
{
    typedef typename Node<KeyType, MaxKeys>::size_type size_type;

    os << '[';

    for (size_type i = 0; i < node.key_count; ++i)
        i ? os << ' ' << node.keys[i] : os << node.keys[i];

    return os << ']';
}

template <class KeyType, std::size_t MaxKeys>
inline static std::ostream &operator<<(std::ostream &os, const IndexNode<KeyType, MaxKeys> &inode)
{
    typedef typename Node<KeyType, MaxKeys>::size_type size_type;

    if (inode.father)
    {
        os << "[<";

        for (size_type i = 0; i < inode.father->key_count; ++i)
            i ? os << ' ' << inode.father->keys[i] : os << inode.father->keys[i];

        os << '>';

        for (size_type i = 0; i < inode.key_count; ++i)
            i ? os << ' ' << inode.keys[i] : os << inode.keys[i];

        return os << ']';
    }
    else
        return os << static_cast<Node<KeyType, MaxKeys>>(inode);
}

template <class KeyType, std::size_t MaxKeys>
inline static std::ostream &operator<<(std::ostream &os, const LeafNode<KeyType, MaxKeys> &lnode)
{
    return os << static_cast<Node<KeyType, MaxKeys>>(lnode);
}
