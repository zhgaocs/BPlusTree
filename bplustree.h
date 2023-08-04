/**
 *  references:
 *  https://segmentfault.com/a/1190000020416577
 *  https://www.cs.usfca.edu/~galles/visualization/BPlusTree.html
 */
#pragma once

#ifndef DEBUG_MODE1
#define DEBUG_MODE1

#include <cstring>
#include <functional>
#include <queue>

#ifdef DEBUG_MODE
#include <iostream>
#endif

// T: operator== operator<=
template <class T, std::size_t Degree>
class BPlusTree
{
public:
    typedef std::size_t size_type;
    typedef T value_type;

public:
    BPlusTree() = default;
    ~BPlusTree();
    bool erase(const value_type &);
    bool find(const value_type &) const;
    void insert(const value_type &);
    void clear();

#ifdef DEBUG_MODE
    void print() const;
#endif

private:
    enum struct BelowType
    {
        INDEX,
        LEAF
    };

    struct Node
    {
        Node() = default;
        virtual ~Node() = default;

#ifdef DEBUG_MODE
        virtual void print() const;
#endif
        value_type k[Degree];
        size_type k_len = 0;
    };

    struct IndexNode : public Node
    {
        IndexNode(BelowType);
        ~IndexNode() = default;

#ifdef DEBUG_MODE
        virtual void print() const override;
#endif

        BelowType b_type;
        Node *b[Degree + 1]; // below nodes
        size_type b_len = 0;

        IndexNode *above = nullptr;
    };

    struct LeafNode : public Node
    {
        LeafNode *next = nullptr;
    };

private:
    static bool _is_in_node(const Node *, const value_type &);
    static size_type _find_pos(const Node *, const value_type &);           // look down
    static size_type _find_pos(const IndexNode *, const Node *);            // look up
    static void _insert(Node *, const value_type &);                        // insert t
    static void _insert(IndexNode *inode, const Node *node, size_type pos); // insert node

public:
    static const size_type npos = -1;

private:
    IndexNode *root = nullptr;
    LeafNode *data = nullptr;
};

template <class T, std::size_t Degree>
inline BPlusTree<T, Degree>::~BPlusTree()
{
    clear();
}

template <class T, std::size_t Degree>
inline bool BPlusTree<T, Degree>::erase(const value_type &)
{
    if (!data)
        return false;
    else if (!root)
        ;
    else
    {
    }
}

template <class T, std::size_t Degree>
inline bool BPlusTree<T, Degree>::find(const value_type &t) const
{
    bool flag = false;

    if (data)
    {
        if (root)
        {
            auto inode = root;
            std::queue<IndexNode *> q;

            while (BelowType::INDEX == inode->b_type)
            {
                q.emplace(inode);
                auto b_pos = _find_pos(inode, t);
                inode = dynamic_cast<IndexNode *>(inode->b[b_pos]);
            }

            auto b_pos = _find_pos(inode, t);
            auto lnode = dynamic_cast<LeafNode *>(inode->b[b_pos]);

            if (_is_in_node(lnode, t))
            {
                flag = true;
#ifdef DEBUG_MODE
                q.emplace(inode);
                while (!q.empty())
                {
                    inode = q.front();

                    inode->print();
                    std::cout << "-->";

                    q.pop();
                }

                lnode->print();
#endif
            }
        }
        else if (_is_in_node(data, t))
        {
            flag = true;
#ifdef DEBUG_MODE
            data->print();
#endif
        }
    }

#ifdef DEBUG_MODE
    if (!flag)
        std::cout << "no this element" << std::endl;
#endif

    return flag;
}

template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::insert(const value_type &t)
{
    if (!data) // insert 1st element
    {
        data = new LeafNode;
        data->k[0] = t;
        data->k_len = 1;
    }
    else if (!root)
    {
        _insert(data, t);

        if (Degree == data->k_len) // need split
        {
            auto lnode = new LeafNode;
            const auto SPLIT_POS = Degree >> 1; // split point

            data->k_len = SPLIT_POS;
            data->next = lnode;

            lnode->k_len = Degree - SPLIT_POS;
            std::memcpy(lnode->k, data->k + SPLIT_POS, sizeof(value_type) * lnode->k_len);

            root = new IndexNode(BelowType::LEAF);
            root->k[0] = lnode->k[0];
            root->k_len = 1;
            root->b[0] = data;
            root->b[1] = lnode;
            root->b_len = 2;
        }
    }
    else
    {
        auto inode = root;
        size_type b_pos;

        while (BelowType::INDEX == inode->b_type)
        {
            b_pos = _find_pos(inode, t);
            inode = dynamic_cast<IndexNode *>(inode->b[b_pos]);
        }

        b_pos = _find_pos(inode, t);
        auto lnode = dynamic_cast<LeafNode *>(inode->b[b_pos]);
        _insert(lnode, t);

        if (Degree == lnode->k_len) // need split
        {
            const auto SPLIT_POS = Degree >> 1;
            auto new_lnode = new LeafNode;

            new_lnode->next = lnode->next;
            new_lnode->k_len = Degree - SPLIT_POS;
            std::memcpy(new_lnode->k, lnode->k + SPLIT_POS, sizeof(value_type) * new_lnode->k_len);

            lnode->k_len = SPLIT_POS;
            lnode->next = new_lnode;

            _insert(inode, new_lnode->k[0]);
            _insert(inode, new_lnode, b_pos + 1);

            // update above index
            while (Degree == inode->k_len)
            {
                bool early_quit = false; // is it possible to withdraw early

                IndexNode *dad_inode = nullptr,
                          *bro_inode = new IndexNode(inode->b_type);

                if (inode->above)
                {
                    dad_inode = inode->above;
                    _insert(dad_inode, inode->k[SPLIT_POS]);
                    _insert(dad_inode, bro_inode, _find_pos(dad_inode, inode) + 1);
                }
                else
                {
                    early_quit = true;

                    dad_inode = new IndexNode(BelowType::INDEX);
                    dad_inode->k[0] = inode->k[SPLIT_POS];
                    dad_inode->k_len = 1;
                    dad_inode->b[0] = inode;
                    dad_inode->b[1] = bro_inode;
                    dad_inode->b_len = 2;

                    inode->above = dad_inode;
                }

                inode->k_len = SPLIT_POS;
                inode->b_len = SPLIT_POS + 1;

                bro_inode->k_len = Degree - SPLIT_POS - 1;
                std::memcpy(bro_inode->k, inode->k + SPLIT_POS + 1, sizeof(value_type) * bro_inode->k_len);
                bro_inode->b_len = bro_inode->k_len + 1;
                std::memcpy(bro_inode->b, inode->b + inode->b_len, sizeof(Node *) * bro_inode->b_len);

                bro_inode->above = dad_inode;

                if (BelowType::INDEX == bro_inode->b_type)
                    for (auto i = 0; i < bro_inode->b_len; ++i)
                        dynamic_cast<IndexNode *>(bro_inode->b[i])->above = bro_inode;

                if (early_quit)
                    break;
                else
                    inode = inode->above;
            }
        }
        else // leafnode not full
            if (b_pos)
                inode->k[b_pos - 1] = lnode->k[0];

        while (inode->above)
            inode = inode->above;

        root = inode;
    }
}

template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::clear()
{
    if (root)
    {
        std::queue<IndexNode *> q;
        q.emplace(root);

        while (!q.empty())
        {
            auto inode = q.front();

            if (BelowType::INDEX == inode->b_type)
                for (auto i = 0; i < inode->b_len; ++i)
                    q.emplace(dynamic_cast<IndexNode *>(inode->b[i]));

            q.pop();
        }

        root = nullptr;
    }
    while (data)
    {
        auto lnode = data;
        data = data->next;
        delete lnode;
    }
    data = nullptr;
}

#ifdef DEBUG_MODE
template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::print() const
{
    // print index nodes
    if (root)
    {
        std::queue<IndexNode *> q;
        std::size_t inode_cnt = 1, while_cnt = 1;
        IndexNode *inode = nullptr;

        q.emplace(root);

        while (!q.empty())
        {
            inode = q.front();
            inode->print();

            if (BelowType::INDEX == inode->b_type)
                for (auto i = 0; i < inode->b_len; ++i)
                    q.emplace(dynamic_cast<IndexNode *>(inode->b[i]));

            q.pop();

            if (inode_cnt == while_cnt)
            {
                inode_cnt += q.size();
                std::cout << '\n';
            }

            ++while_cnt;
        }
    }

    // print leafnodes(data)
    auto lnode = data;
    while (lnode)
    {
        lnode->print();
        lnode = lnode->next;

        if (lnode)
            std::cout << "->";
    }
    std::cout << std::endl;
}
#endif

template <class T, std::size_t Degree>
inline BPlusTree<T, Degree>::IndexNode::IndexNode(BelowType btype) : b_type(btype)
{
}

#ifdef DEBUG_MODE
template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::Node::print() const
{
    std::cout << '[';

    for (auto i = 0; i < k_len; ++i)
        i != 0 ? std::cout << ' ' << k[i] : std::cout << k[i];

    std::cout << ']';
}

template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::IndexNode::print() const
{
    if (above)
    {
        std::cout << '[';

        // print current inode's father inode
        std::cout << '<';
        for (auto i = 0; i < above->k_len; ++i)
            i != 0 ? std::cout << ' ' << above->k[i] : std::cout << above->k[i];
        std::cout << '>';

        // print current indoe
        for (auto i = 0; i < this->k_len; ++i)
            i != 0 ? std::cout << ' ' << this->k[i] : std::cout << this->k[i];

        std::cout << ']';
    }
    else
        Node::print();
}
#endif

template <class T, std::size_t Degree>
inline bool BPlusTree<T, Degree>::_is_in_node(const Node *node, const value_type &t)
{
    for (auto i = 0; i < node->k_len; ++i)
        if (std::equal_to<value_type>()(t, node->k[i]))
            return true;
    return false;
}

template <class T, std::size_t Degree>
inline typename BPlusTree<T, Degree>::size_type
BPlusTree<T, Degree>::_find_pos(const Node *node, const value_type &t)
{
    auto pos = 0;

    while (std::less_equal<value_type>()(node->k[pos], t) && pos < node->k_len)
        ++pos;

    return pos;
}

template <class T, std::size_t Degree>
inline typename BPlusTree<T, Degree>::size_type
BPlusTree<T, Degree>::_find_pos(const IndexNode *inode, const Node *node)
{
    for (auto pos = 0; pos < inode->b_len; ++pos)
        if (inode->b[pos] == node)
            return pos;

    return npos;
}

template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::_insert(Node *node, const value_type &t)
{
    auto pos = _find_pos(node, t);

    for (auto i = node->k_len; i > pos; --i)
        node->k[i] = node->k[i - 1];

    node->k[pos] = t;

    ++node->k_len;
}

template <class T, std::size_t Degree>
inline void BPlusTree<T, Degree>::_insert(IndexNode *inode, const Node *node, size_type pos)
{
    for (auto i = inode->b_len; i > pos; --i)
        inode->b[i] = inode->b[i - 1];

    inode->b[pos] = const_cast<Node *>(node);

    ++inode->b_len;
}

#endif