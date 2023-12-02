#ifndef UTILS_H
#define UITLS_H 1

#include "def.h"

/**
 * T must overload operator== and operator<=
 * arr is ascending ordered
 */

template <class T>
inline size_type locate_value(const T *arr, size_type len, const T &value)
{
    for (size_type i = 0; i < len; ++i)
        if (value == arr[i])
            return i;

    return -1; // can not find
}

template <class T>
inline size_type locate_insert(const T *arr, size_type len, const T &value)
{
    size_type pos = 0;

    while (pos < len && arr[pos] <= value)
        ++pos;

    return pos;
}

template <class T>
inline void insert_at(T *arr, size_type &len, const T &value, size_type pos)
{
    for (size_type i = len; i > pos; --i)
        arr[i] = arr[i - 1];

    arr[pos] = value;

    ++len;
}

template <class T>
inline void insert_value(T *arr, size_type &len, const T &value)
{
    insert_at(arr, len, value, locate_insert(arr, len, value));
}

template <class T>
inline void remove_at(T *arr, size_type &len, size_type pos)
{
    for (size_type i = pos + 1; i < len; ++i)
        arr[i - 1] = arr[i];

    --len;
}

template <class T>
inline bool remove_value(T *arr, size_type &len, const T &value)
{
    size_type pos = locate_value(arr, len, value);
    if (size_type(-1) != pos)
    {
        for (size_type i = pos + 1; i < len; ++i)
            arr[i - 1] = arr[i];

        --len;
        return true;
    }
    else
        return false;
}

#endif
