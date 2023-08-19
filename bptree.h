#ifndef BPTREE_H
#define BPTREE_H

#include <cstring>
#include <queue>
#include <istream>
#include "node.h"
#include "utils.h"

// KeyType must overload operator== and operator<=
// Degree >= 3
template <class KeyType, size_type Degree>
class BPlusTree
{
    friend std::ostream &operator<<(std::ostream &os, const BPlusTree &bpt)
    {
        bpt.serialization_to(os);
        return os;
    }

public:
    typedef KeyType key_type;
    typedef size_type size_type;

protected:
    typedef IndexNode<key_type, Degree> INode;
    typedef LeafNode<key_type, Degree> LNode;

public:
    BPlusTree() = default;
    BPlusTree(const BPlusTree &); // TODO
    BPlusTree(BPlusTree &&) noexcept;
    BPlusTree &operator=(const BPlusTree &); // TODO
    BPlusTree &operator=(BPlusTree &&) noexcept;
    ~BPlusTree() noexcept;

public:
    bool deserialization_from(const std::istream &);
    bool find(const key_type &) const;
    bool remove(const key_type &);
    void clear() noexcept;
    void insert(const key_type &);
    void serialization_to(std::ostream &) const;

protected:
    INode *root = nullptr;
    LNode *data = nullptr;
};

template <class KeyType, size_type Degree>
inline BPlusTree<KeyType, Degree>::BPlusTree(BPlusTree &&other) noexcept
    : root(other.root), data(other.data)
{
    other.root = nullptr;
    other.data = nullptr;
}

template <class KeyType, size_type Degree>
inline BPlusTree<KeyType, Degree> &BPlusTree<KeyType, Degree>::operator=(BPlusTree &&other) noexcept
{
    if (this != &other)
    {
        clear();
        root = other.root;
        data = other.data;
        other.root = nullptr;
        other.data = nullptr;
    }
    return *this;
}

template <class KeyType, size_type Degree>
inline BPlusTree<KeyType, Degree>::~BPlusTree() noexcept
{
    clear();
}

template <class KeyType, size_type Degree>
inline bool BPlusTree<KeyType, Degree>::deserialization_from(const std::istream &)
{
    return false;
}

template <class KeyType, size_type Degree>
inline bool BPlusTree<KeyType, Degree>::find(const key_type &k) const
{
    if (data)
    {
        if (root)
        {
            INode *inode = root;
            LNode *lnode = nullptr;
            size_type child_idx;

            while (ChildType::INDEX == inode->child_type)
            {
                child_idx = locate_insert(inode->keys, inode->key_count, k);

                if (child_idx && k == inode->keys[child_idx - 1]) // if indexnode has keyword k
                    return true;

                inode = static_cast<INode *>(inode->children[child_idx]);
            }
            child_idx = locate_insert(inode->keys, inode->key_count, k);

            if (child_idx && k == inode->keys[child_idx - 1]) // if indexnode has keyword k
                return true;

            lnode = static_cast<LNode *>(inode->children[child_idx]);
            return size_type(-1) != locate_value(lnode->keys, lnode->key_count, k);
        }
        else // there is no indexnodes
            return size_type(-1) != locate_value(data->keys, data->key_count, k);
    }
    else
        return false;
}

template <class KeyType, size_type Degree>
inline bool BPlusTree<KeyType, Degree>::remove(const key_type &k)
{
    if (!data)
        return false;
    else if (!root)
    {
        bool flag = remove_value(data->keys, data->key_count, k);

        if (!data->key_count)
        {
            delete data;
            data = nullptr;
        }

        return flag;
    }
    else
    {
        INode *inode = root;
        LNode *lnode = nullptr;
        size_type child_idx, k_idx;

        while (ChildType::INDEX == inode->child_type)
        {
            child_idx = locate_insert(inode->keys, inode->key_count, k);
            inode = static_cast<INode *>(inode->children[child_idx]);
        }

        child_idx = locate_insert(inode->keys, inode->key_count, k);
        lnode = static_cast<LNode *>(inode->children[child_idx]);
        k_idx = locate_value(lnode->keys, lnode->key_count, k);

        if (size_type(-1) == k_idx) // can not find k
            return false;
        else // find k
        {
            const size_type NODE_MIN_LEN = Degree & 1 ? Degree >> 1 : (Degree >> 1) - 1;
            remove_at(lnode->keys, lnode->key_count, k_idx);

            if (lnode->key_count < NODE_MIN_LEN) // lnode borrow or merge
            {
                size_type bro_idx;
                LNode *bro_lnode = nullptr;

                if (!child_idx)
                {
                    bro_idx = 1;
                    bro_lnode = lnode->next;
                }
                else
                {
                    bro_idx = child_idx - 1;
                    bro_lnode = static_cast<LNode *>(inode->children[bro_idx]);

                    if (child_idx != inode->key_count && bro_lnode->key_count == NODE_MIN_LEN && lnode->next->key_count != NODE_MIN_LEN)
                    {
                        bro_idx = child_idx + 1;
                        bro_lnode = lnode->next;
                    }
                }

                if (bro_lnode->key_count != NODE_MIN_LEN) // lnode borrow
                {
                    if (bro_idx < child_idx)
                    {
                        --bro_lnode->key_count;
                        insert_at(lnode->keys, lnode->key_count, bro_lnode->keys[bro_lnode->key_count], 0);
                    }
                    else
                    {
                        lnode->keys[lnode->key_count] = bro_lnode->keys[0];
                        ++lnode->key_count;
                        remove_at(bro_lnode->keys, bro_lnode->key_count, 0);
                    }
                }
                else // lnode merge
                {
                    // Determine the value of bro_inode
                    if (bro_idx < child_idx)
                    {
                        std::memcpy(bro_lnode->keys + bro_lnode->key_count, lnode->keys,
                                    sizeof(key_type) * lnode->key_count);
                        bro_lnode->key_count += lnode->key_count;
                        bro_lnode->next = lnode->next;

                        delete lnode;
                        lnode = bro_lnode;

                        remove_at(inode->keys, inode->key_count, bro_idx);
                        remove_at(inode->children, inode->child_count, child_idx);
                    }
                    else
                    {
                        std::memcpy(lnode->keys + lnode->key_count, bro_lnode->keys,
                                    sizeof(key_type) * bro_lnode->key_count);
                        lnode->key_count += bro_lnode->key_count;
                        lnode->next = bro_lnode->next;

                        delete bro_lnode;

                        remove_at(inode->keys, inode->key_count, child_idx);
                        remove_at(inode->children, inode->child_count, bro_idx);
                    }

                    if (!child_idx && !k_idx) // update ancestor inode
                    {
                        INode *dad_inode = nullptr, *inode_cpy = inode;

                        while (dad_inode = inode->father)
                        {
                            if (static_cast<INode *>(dad_inode->children[0]) == inode)
                                inode = dad_inode;
                            else
                            {
                                child_idx = locate_value(dad_inode->children, dad_inode->child_count,
                                                         static_cast<Node<KeyType, Degree> *>(inode));
                                inode = dad_inode;
                                break;
                            }
                        }

                        if (child_idx)
                            inode->keys[child_idx - 1] = lnode->keys[0];
                        inode = inode_cpy;
                    }

                    while (inode != root && inode->key_count < NODE_MIN_LEN)
                    {
                        INode *dad_inode = inode->father, *bro_inode = nullptr;
                        child_idx = locate_value(dad_inode->children, dad_inode->child_count,
                                                 static_cast<Node<key_type, Degree> *>(inode));

                        if (!child_idx)
                        {
                            bro_idx = 1;
                            bro_inode = static_cast<INode *>(dad_inode->children[bro_idx]);
                        }
                        else
                        {
                            bro_idx = child_idx - 1;
                            bro_inode = static_cast<INode *>(dad_inode->children[bro_idx]);

                            if (child_idx != dad_inode->key_count && bro_inode->key_count == NODE_MIN_LEN && dad_inode->children[child_idx + 1]->key_count != NODE_MIN_LEN)
                            {
                                bro_idx = child_idx + 1;
                                bro_inode = static_cast<INode *>(dad_inode->children[bro_idx]);
                            }
                        }

                        if (bro_inode->key_count != NODE_MIN_LEN) // inode borrow
                            if (bro_idx < child_idx)              // borrow left
                            {
                                insert_at(inode->keys, inode->key_count, dad_inode->keys[bro_idx], 0);

                                --bro_inode->key_count;
                                dad_inode->keys[bro_idx] = bro_inode->keys[bro_inode->key_count];

                                --bro_inode->child_count;
                                if (ChildType::INDEX == bro_inode->child_type)
                                    static_cast<INode *>(bro_inode->children[bro_inode->child_count])->father = inode;
                                insert_at(inode->children, inode->child_count, bro_inode->children[bro_inode->child_count], 0);
                            }
                            else // borrow right
                            {
                                inode->keys[inode->key_count] = dad_inode->keys[child_idx];
                                ++inode->key_count;

                                dad_inode->keys[child_idx] = bro_inode->keys[0];

                                if (ChildType::INDEX == bro_inode->child_type)
                                    static_cast<INode *>(bro_inode->children[0])->father = inode;
                                inode->children[inode->child_count] = bro_inode->children[0];
                                ++inode->child_count;

                                remove_at(bro_inode->keys, bro_inode->key_count, 0);
                                remove_at(bro_inode->children, bro_inode->child_count, 0);
                            }

                        else                         // inode merge
                            if (bro_idx < child_idx) // merge left
                            {
                                bro_inode->keys[bro_inode->key_count] = dad_inode->keys[bro_idx];
                                ++bro_inode->key_count;
                                std::memcpy(bro_inode->keys + bro_inode->key_count, inode->keys, sizeof(key_type) * inode->key_count);
                                bro_inode->key_count += inode->key_count;

                                if (ChildType::INDEX == inode->child_type)
                                    for (size_type i = 0; i < inode->child_count; ++i)
                                        static_cast<INode *>(inode->children[i])->father = bro_inode;

                                std::memcpy(bro_inode->children + bro_inode->child_count, inode->children, sizeof(Node<key_type, Degree> *) * inode->child_count);
                                bro_inode->child_count += inode->child_count;

                                remove_at(dad_inode->keys, dad_inode->key_count, bro_idx);
                                remove_at(dad_inode->children, dad_inode->child_count, child_idx);

                                delete inode;
                                inode = bro_inode;
                            }
                            else // merge right
                            {
                                inode->keys[inode->key_count] = dad_inode->keys[child_idx];
                                ++inode->key_count;
                                std::memcpy(inode->keys + inode->key_count, bro_inode->keys, sizeof(key_type) * bro_inode->key_count);
                                inode->key_count += bro_inode->key_count;

                                if (ChildType::INDEX == bro_inode->child_type)
                                    for (size_type i = 0; i < bro_inode->child_count; ++i)
                                        static_cast<INode *>(bro_inode->children[i])->father = inode;

                                std::memcpy(inode->children + inode->child_count, bro_inode->children, sizeof(Node<key_type, Degree> *) * bro_inode->child_count);
                                inode->child_count += bro_inode->child_count;

                                remove_at(dad_inode->keys, dad_inode->key_count, child_idx);
                                remove_at(dad_inode->children, dad_inode->child_count, bro_idx);

                                delete bro_inode;
                            }

                        inode = dad_inode;
                    }

                    if (!root->key_count)
                    {
                        if (ChildType::INDEX == root->child_type)
                        {
                            inode = root;
                            root = static_cast<INode *>(root->children[0]);
                            root->father = nullptr;
                            delete inode;
                        }
                        else
                        {
                            delete root;
                            root = nullptr;
                        }
                    }

                    return true;
                }
            }

            // update some index
            if (child_idx)
                inode->keys[child_idx - 1] = lnode->keys[0]; // update direct father inode
            else
            {
                inode->keys[0] = lnode->next->keys[0]; // update direct father inode
                if (!k_idx)                            // update ancestor inode
                {
                    INode *dad_inode = nullptr;

                    while (dad_inode = inode->father)
                    {
                        if (static_cast<INode *>(dad_inode->children[0]) == inode)
                            inode = dad_inode;
                        else
                        {
                            child_idx = locate_value(dad_inode->children, dad_inode->child_count,
                                                     static_cast<Node<KeyType, Degree> *>(inode));
                            inode = dad_inode;
                            break;
                        }
                    }

                    if (child_idx)
                        inode->keys[child_idx - 1] = lnode->keys[0];
                }
            }

            return true;
        }
    }
}

template <class KeyType, size_type Degree>
inline void BPlusTree<KeyType, Degree>::clear() noexcept
{
    if (root)
    {
        INode *inode;
        std::queue<INode *> q;
        q.emplace(root);

        while (!q.empty())
        {
            inode = q.front();

            if (ChildType::INDEX == inode->child_type)
                for (size_type i = 0; i < inode->child_count; ++i)
                    q.emplace(static_cast<INode *>(inode->children[i]));

            q.pop();
            delete inode;
        }

        root = nullptr;
    }
    if (data)
    {
        LNode *lnode = data->next;
        delete data;

        while (lnode)
        {
            data = lnode->next;
            delete lnode;
            lnode = data;
        }

        data = nullptr;
    }
}

template <class KeyType, size_type Degree>
inline void BPlusTree<KeyType, Degree>::insert(const key_type &k)
{
    if (!data) // there is no keys
    {
        data = new LNode;
        data->keys[0] = k;
        data->key_count = 1;
    }
    else if (!root) // there is no indexnodes
    {
        insert_value(data->keys, data->key_count, k);

        if (Degree == data->key_count) // need split
        {
            const size_type SPLIT_POS = Degree >> 1; // split point
            LNode *bro_lnode = new LNode;

            data->key_count = SPLIT_POS;
            data->next = bro_lnode;

            bro_lnode->key_count = Degree - SPLIT_POS;
            std::memcpy(bro_lnode->keys, data->keys + SPLIT_POS, sizeof(key_type) * bro_lnode->key_count);

            root = new INode(ChildType::LEAF);
            root->keys[0] = bro_lnode->keys[0];
            root->key_count = 1;
            root->children[0] = data;
            root->children[1] = bro_lnode;
            root->child_count = 2;
        }
    }
    else
    {
        INode *inode = root;
        LNode *lnode = nullptr;
        size_type child_idx;

        while (ChildType::INDEX == inode->child_type)
        {
            child_idx = locate_insert(inode->keys, inode->key_count, k);
            inode = static_cast<INode *>(inode->children[child_idx]);
        }

        child_idx = locate_insert(inode->keys, inode->key_count, k);
        lnode = static_cast<LNode *>(inode->children[child_idx]);
        insert_value(lnode->keys, lnode->key_count, k);

        if (Degree == lnode->key_count) // need split
        {
            const size_type SPLIT_POS = Degree >> 1;
            LNode *bro_lnode = new LNode; // right brother leafnode

            bro_lnode->next = lnode->next;
            bro_lnode->key_count = Degree - SPLIT_POS;
            std::memcpy(bro_lnode->keys, lnode->keys + SPLIT_POS, sizeof(key_type) * bro_lnode->key_count);

            lnode->key_count = SPLIT_POS;
            lnode->next = bro_lnode;

            insert_at(inode->keys, inode->key_count, bro_lnode->keys[0], child_idx);
            insert_at(inode->children, inode->child_count,
                      static_cast<Node<key_type, Degree> *>(bro_lnode), child_idx + 1);

            while (Degree == inode->key_count) // father indexnodes need split
            {
                bool exit_loop = false;
                INode *dad_inode = nullptr, *bro_inode = new INode(inode->child_type);

                if (inode->father)
                {
                    dad_inode = inode->father;
                    child_idx = locate_value(dad_inode->children, dad_inode->child_count, static_cast<Node<KeyType, Degree> *>(inode));

                    insert_at(dad_inode->keys, dad_inode->key_count, inode->keys[SPLIT_POS], child_idx);
                    insert_at(dad_inode->children, dad_inode->child_count,
                              static_cast<Node<KeyType, Degree> *>(bro_inode), child_idx + 1);
                }
                else
                {
                    exit_loop = true;
                    dad_inode = new INode(ChildType::INDEX);

                    dad_inode->keys[0] = inode->keys[SPLIT_POS];
                    dad_inode->key_count = 1;
                    dad_inode->children[0] = inode;
                    dad_inode->children[1] = bro_inode;
                    dad_inode->child_count = 2;

                    inode->father = dad_inode;
                }

                inode->key_count = SPLIT_POS;
                inode->child_count = SPLIT_POS + 1;

                bro_inode->key_count = Degree - SPLIT_POS - 1;
                std::memcpy(bro_inode->keys, inode->keys + SPLIT_POS + 1,
                            sizeof(key_type) * bro_inode->key_count);
                bro_inode->child_count = bro_inode->key_count + 1;
                std::memcpy(bro_inode->children, inode->children + inode->child_count,
                            sizeof(Node<key_type, Degree> *) * bro_inode->child_count);
                bro_inode->father = dad_inode;

                if (ChildType::INDEX == bro_inode->child_type)
                    for (size_type i = 0; i < bro_inode->child_count; ++i)
                        static_cast<INode *>(bro_inode->children[i])->father = bro_inode;

                if (exit_loop)
                    break;
                else
                    inode = inode->father;
            }
        }
        else // Degree != lnode->key_count, do not need split
            if (child_idx)
                inode->keys[child_idx - 1] = lnode->keys[0];

        while (inode->father)
            inode = inode->father;

        root = inode;
    }
}

template <class KeyType, size_type Degree>
inline void BPlusTree<KeyType, Degree>::serialization_to(std::ostream &os) const
{
    // print indexnodes
    if (root)
    {
        std::queue<INode *> q;
        size_type inode_cnt = 1, while_cnt = 1;
        INode *inode = nullptr;

        q.emplace(root);

        while (!q.empty())
        {
            inode = q.front();

            if (ChildType::INDEX == inode->child_type)
                for (size_type i = 0; i < inode->child_count; ++i)
                    q.emplace(static_cast<INode *>(inode->children[i]));

            q.pop();

            os << *inode;
            if (inode_cnt == while_cnt)
            {
                os << '\n';
                inode_cnt += q.size();
            }

            ++while_cnt;
        }
    }

    // print leafnodes
    if (data)
    {
        LNode *lnode = data->next;
        os << *data;

        while (lnode)
        {
            os << "->" << *lnode;
            lnode = lnode->next;
        }
    }
    else
        os << "[]";
}

#endif